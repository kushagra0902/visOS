#pragma once

#include "core/kernel_event.hpp"
#include "core/cpu_state.hpp"
#include "core/process_control_block.hpp"
#include "core/syscall_profile.hpp"

#include <functional>
#include <memory>

namespace visos {

// CPU does not know about the Kernel. It raises events via this callback.
using EventCallback = std::function<void(KernelEvent, ProcessControlBlock*)>;

class CPU {
public:
    explicit CPU(EventCallback on_event);

    // Execute one clock tick. May raise events via the callback.
    void executeTick();

    void loadProcess(std::shared_ptr<ProcessControlBlock> pcb,
                     const SyscallProfile& profile, int quantum);
    std::shared_ptr<ProcessControlBlock> unloadProcess();

    void setQuantum(int quantum) noexcept { remaining_quantum_ = quantum; }

    [[nodiscard]] CPUState state() const noexcept { return state_; }
    [[nodiscard]] const ProcessControlBlock* currentProcess() const noexcept;
    [[nodiscard]] int remainingQuantum() const noexcept { return remaining_quantum_; }

private:
    void raiseEvent(KernelEvent event);

    EventCallback on_event_;
    std::shared_ptr<ProcessControlBlock> current_process_;
    SyscallProfile current_profile_;
    int remaining_quantum_ = 0;
    CPUState state_ = CPUState::Idle;
};

} // namespace visos
