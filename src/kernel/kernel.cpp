#include "kernel/kernel.hpp"
#include "util/logger.hpp"

#include <stdexcept>

namespace visos {

Kernel::Kernel(Config config)
    : config_(config)
{
    // Create Scheduler with reference to MemoryManager
    scheduler_ = std::make_unique<Scheduler>(
        config_.policy, config_.time_quantum, memory_manager_);

    // Create CPU with event callback pointing back to this Kernel
    cpu_ = std::make_unique<CPU>(
        [this](KernelEvent event, ProcessControlBlock* pcb) {
            handleEvent(event, pcb);
        });
}

ProgramID Kernel::submitProgram(ProgramMetadata metadata) {
    return program_store_.store(std::move(metadata));
}

void Kernel::runProgram(ProgramID id) {
    auto meta_opt = program_store_.load(id);
    if (!meta_opt.has_value()) {
        throw std::runtime_error("Program ID " + std::to_string(id) + " not found");
    }

    auto& meta = meta_opt.value();
    PID pid = pid_generator_.next();

    auto pcb = std::make_shared<ProcessControlBlock>(
        pid, meta.required_resources, meta.estimated_cpu_time);

    Logger::logProcessCreated(pid, meta.program_name);
    notifyProcessCreated(*pcb);

    // Store metadata for later use (syscall profile lookup during dispatch)
    process_metadata_.emplace(pid, meta);

    // NEW -> READY (admit)
    transitionState(*pcb, StateTransition::Admit);

    // Add to ready queue
    if (config_.policy == SchedulingPolicy::Priority) {
        memory_manager_.enqueueReadyByPriority(pcb);
    } else {
        memory_manager_.enqueueReady(pcb);
    }

    all_processes_.push_back(std::move(pcb));
}

void Kernel::notifyTick(Tick current_tick) {
    current_tick_ = current_tick;
    Logger::logTick(current_tick);

    // Check for I/O completions: wake up blocked processes
    checkBlockedCompletions();

    // CPU executes one tick (may raise events via callback)
    cpu_->executeTick();

    notifyTickCompleted(current_tick);
}

bool Kernel::hasActiveProcesses() const noexcept {
    for (const auto& pcb : all_processes_) {
        if (pcb->state() != ProcessState::Terminated) {
            return true;
        }
    }
    return false;
}

const std::vector<std::shared_ptr<ProcessControlBlock>>&
Kernel::allProcesses() const noexcept {
    return all_processes_;
}

const MemoryManager& Kernel::memoryManager() const noexcept {
    return memory_manager_;
}

const CPU& Kernel::cpu() const noexcept {
    return *cpu_;
}

void Kernel::addObserver(std::shared_ptr<IKernelObserver> observer) {
    observers_.push_back(std::move(observer));
}

// --- Event Handling (from CPU callback) ---

void Kernel::handleEvent(KernelEvent event, ProcessControlBlock* pcb) {
    if (pcb) {
        Logger::logEvent(current_tick_, event, pcb->pid());
    }

    switch (event) {
        case KernelEvent::SyscallBlock:
            handleSyscallBlock(*pcb);
            break;
        case KernelEvent::QuantumExpired:
            handleQuantumExpired(*pcb);
            break;
        case KernelEvent::ProcessExit:
            handleProcessExit(*pcb);
            break;
        case KernelEvent::CpuIdle:
            handleCpuIdle();
            break;
    }
}

void Kernel::handleSyscallBlock(ProcessControlBlock& pcb) {
    // Look up the syscall that fired at this tick
    auto it = process_metadata_.find(pcb.pid());
    Tick cpu_time = pcb.cpuTimeUsed();
    Syscall syscall{SyscallType::IoRead, 3}; // default

    if (it != process_metadata_.end()) {
        auto sc = it->second.syscall_profile.getSyscallAt(cpu_time);
        if (sc.has_value()) {
            syscall = sc.value();
        }
    }

    // Delegate to SyscallHandler
    auto result = syscall_handler_.handle(syscall, pcb);

    // Transition RUNNING -> BLOCKED
    transitionState(pcb, StateTransition::BlockingSyscall);

    // Move to blocked queue with wakeup time
    Tick wakeup_tick = current_tick_ + syscall.duration;
    auto unloaded = cpu_->unloadProcess();
    memory_manager_.moveToBlocked(std::move(unloaded), wakeup_tick);

    // Notify scheduler and dispatch next
    notifySchedulerInvoked(KernelEvent::SyscallBlock);
    scheduler_->onProcessBlocked(nullptr);
    dispatchNext();
}

void Kernel::handleQuantumExpired(ProcessControlBlock& pcb) {
    // Transition RUNNING -> READY
    transitionState(pcb, StateTransition::QuantumExpired);

    auto unloaded = cpu_->unloadProcess();

    // Re-enqueue in ready queue
    if (config_.policy == SchedulingPolicy::Priority) {
        memory_manager_.enqueueReadyByPriority(std::move(unloaded));
    } else {
        memory_manager_.enqueueReady(std::move(unloaded));
    }

    // Select and dispatch next
    notifySchedulerInvoked(KernelEvent::QuantumExpired);
    dispatchNext();
}

void Kernel::handleProcessExit(ProcessControlBlock& pcb) {
    // Transition RUNNING -> TERMINATED
    transitionState(pcb, StateTransition::Exit);

    cpu_->unloadProcess();

    notifySchedulerInvoked(KernelEvent::ProcessExit);
    scheduler_->onProcessExit(nullptr);

    // Try to dispatch next process
    dispatchNext();

    // Check if simulation is complete
    if (!hasActiveProcesses()) {
        Logger::logMessage("All processes terminated. Simulation complete.");
        notifySimulationComplete();
    }
}

void Kernel::handleCpuIdle() {
    // Try to dispatch a process from the ready queue
    dispatchNext();
}

void Kernel::dispatchNext() {
    auto next = scheduler_->selectNextProcess();
    if (!next) {
        Logger::logMessage("No process in ready queue. CPU idle.");
        return;
    }

    PID old_pid = 0;
    if (cpu_->currentProcess()) {
        old_pid = cpu_->currentProcess()->pid();
    }

    // Transition READY -> RUNNING
    transitionState(*next, StateTransition::Dispatch);

    PID new_pid = next->pid();

    // Look up syscall profile for this process
    SyscallProfile profile;
    auto it = process_metadata_.find(new_pid);
    if (it != process_metadata_.end()) {
        profile = it->second.syscall_profile;
    }

    cpu_->loadProcess(std::move(next), profile, scheduler_->defaultQuantum());

    if (old_pid != 0) {
        Logger::logContextSwitch(old_pid, new_pid);
        notifyContextSwitch(old_pid, new_pid);
    }
}

void Kernel::checkBlockedCompletions() {
    auto completed = memory_manager_.checkBlockedCompletions(current_tick_);
    for (auto& pcb : completed) {
        // Transition BLOCKED -> READY
        transitionState(*pcb, StateTransition::IoComplete);

        if (config_.policy == SchedulingPolicy::Priority) {
            memory_manager_.enqueueReadyByPriority(std::move(pcb));
        } else {
            memory_manager_.enqueueReady(std::move(pcb));
        }
    }
}

void Kernel::transitionState(ProcessControlBlock& pcb, StateTransition transition) {
    ProcessState from = pcb.state();
    pcb.applyTransition(transition);
    ProcessState to = pcb.state();
    Logger::logStateTransition(pcb.pid(), from, to);
    notifyStateTransition(pcb.pid(), from, to);
}

// --- Observer notifications ---

void Kernel::notifyTickCompleted(Tick tick) {
    for (auto& obs : observers_) { obs->onTickCompleted(tick); }
}

void Kernel::notifyProcessCreated(const ProcessControlBlock& pcb) {
    for (auto& obs : observers_) { obs->onProcessCreated(pcb); }
}

void Kernel::notifyStateTransition(PID pid, ProcessState from, ProcessState to) {
    for (auto& obs : observers_) { obs->onStateTransition(pid, from, to); }
}

void Kernel::notifyContextSwitch(PID old_pid, PID new_pid) {
    for (auto& obs : observers_) { obs->onContextSwitch(old_pid, new_pid); }
}

void Kernel::notifySchedulerInvoked(KernelEvent reason) {
    for (auto& obs : observers_) { obs->onSchedulerInvoked(reason); }
}

void Kernel::notifySimulationComplete() {
    for (auto& obs : observers_) { obs->onSimulationComplete(); }
}

} // namespace visos
