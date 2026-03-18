#include "services/cpu.hpp"

namespace visos {

CPU::CPU(EventCallback on_event)
    : on_event_(std::move(on_event))
{
}

void CPU::executeTick() {
    if (state_ == CPUState::Idle) {
        raiseEvent(KernelEvent::CpuIdle);
        return;
    }

    // Advance execution: increment CPU time used
    current_process_->incrementCpuTime(1);
    remaining_quantum_--;

    // Save simulated register state (advance program counter)
    auto regs = current_process_->registers();
    regs.program_counter++;
    current_process_->setRegisters(regs);

    // Check for blocking syscall at this tick
    Tick cpu_time = current_process_->cpuTimeUsed();
    auto syscall = current_profile_.getSyscallAt(cpu_time);
    if (syscall.has_value()) {
        raiseEvent(KernelEvent::SyscallBlock);
        return;
    }

    // Check if process has finished all its work
    if (current_process_->isFinished()) {
        raiseEvent(KernelEvent::ProcessExit);
        return;
    }

    // Check if quantum has expired
    if (remaining_quantum_ <= 0) {
        raiseEvent(KernelEvent::QuantumExpired);
        return;
    }

    // No event: continue execution next tick
}

void CPU::loadProcess(std::shared_ptr<ProcessControlBlock> pcb,
                      const SyscallProfile& profile, int quantum) {
    current_process_ = std::move(pcb);
    current_profile_ = profile;
    remaining_quantum_ = quantum;
    state_ = CPUState::Executing;
}

std::shared_ptr<ProcessControlBlock> CPU::unloadProcess() {
    auto pcb = std::move(current_process_);
    current_process_ = nullptr;
    current_profile_ = SyscallProfile{};
    remaining_quantum_ = 0;
    state_ = CPUState::Idle;
    return pcb;
}

const ProcessControlBlock* CPU::currentProcess() const noexcept {
    return current_process_.get();
}

void CPU::raiseEvent(KernelEvent event) {
    on_event_(event, current_process_.get());
}

} // namespace visos
