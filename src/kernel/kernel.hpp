#pragma once

#include "core/kernel_event.hpp"
#include "core/program_metadata.hpp"
#include "core/process_control_block.hpp"
#include "core/scheduling_policy.hpp"
#include "core/types.hpp"

#include "services/cpu.hpp"
#include "services/scheduler.hpp"
#include "services/memory_manager.hpp"
#include "services/syscall_handler.hpp"
#include "services/program_store.hpp"

#include "util/pid_generator.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace visos {

// Observer interface for UI layer decoupling
class IKernelObserver {
public:
    virtual ~IKernelObserver() = default;
    virtual void onTickCompleted(Tick tick) = 0;
    virtual void onProcessCreated(const ProcessControlBlock& pcb) = 0;
    virtual void onStateTransition(PID pid, ProcessState from, ProcessState to) = 0;
    virtual void onContextSwitch(PID old_pid, PID new_pid) = 0;
    virtual void onSchedulerInvoked(KernelEvent reason) = 0;
    virtual void onSimulationComplete() = 0;
};

class Kernel {
public:
    struct Config {
        SchedulingPolicy policy = SchedulingPolicy::RoundRobin;
        int time_quantum = 4;
    };

    explicit Kernel(Config config);

    // --- Public API (called by UI / CLI) ---
    ProgramID submitProgram(ProgramMetadata metadata);
    void runProgram(ProgramID id);

    // Called by SimulationEngine on each clock tick
    void notifyTick(Tick current_tick);

    // Observer registration
    void addObserver(std::shared_ptr<IKernelObserver> observer);

    // --- Snapshot queries ---
    [[nodiscard]] const std::vector<std::shared_ptr<ProcessControlBlock>>&
        allProcesses() const noexcept;

    [[nodiscard]] bool hasActiveProcesses() const noexcept;
    [[nodiscard]] const MemoryManager& memoryManager() const noexcept;
    [[nodiscard]] const CPU& cpu() const noexcept;
    [[nodiscard]] Tick currentTick() const noexcept { return current_tick_; }

private:
    // Event handler - invoked by CPU callback
    void handleEvent(KernelEvent event, ProcessControlBlock* pcb);

    // Internal orchestration
    void handleSyscallBlock(ProcessControlBlock& pcb);
    void handleQuantumExpired(ProcessControlBlock& pcb);
    void handleProcessExit(ProcessControlBlock& pcb);
    void handleCpuIdle();
    void dispatchNext();
    void checkBlockedCompletions();

    void transitionState(ProcessControlBlock& pcb, StateTransition transition);

    // Notify all observers
    void notifyTickCompleted(Tick tick);
    void notifyProcessCreated(const ProcessControlBlock& pcb);
    void notifyStateTransition(PID pid, ProcessState from, ProcessState to);
    void notifyContextSwitch(PID old_pid, PID new_pid);
    void notifySchedulerInvoked(KernelEvent reason);
    void notifySimulationComplete();

    // Owned services
    MemoryManager memory_manager_;
    std::unique_ptr<Scheduler> scheduler_;
    std::unique_ptr<CPU> cpu_;
    SyscallHandler syscall_handler_;
    ProgramStore program_store_;
    PidGenerator pid_generator_;

    Config config_;
    Tick current_tick_ = 0;

    // Master process list and metadata mapping
    std::vector<std::shared_ptr<ProcessControlBlock>> all_processes_;
    std::unordered_map<PID, ProgramMetadata> process_metadata_;

    std::vector<std::shared_ptr<IKernelObserver>> observers_;
};

} // namespace visos
