#pragma once

#include "core/kernel_event.hpp"
#include "core/syscall.hpp"
#include "core/process_control_block.hpp"

#include <memory>

namespace visos {

struct SyscallResult {
    KernelEvent event;
    Syscall syscall; // the syscall that triggered it (for I/O duration)
};

class SyscallHandler {
public:
    // Returns the kernel event produced by handling this syscall.
    // All syscalls in this simulation are blocking.
    [[nodiscard]] SyscallResult handle(const Syscall& syscall,
                                       const ProcessControlBlock& pcb) const;
};

} // namespace visos
