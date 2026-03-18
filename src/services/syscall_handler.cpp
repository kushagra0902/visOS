#include "services/syscall_handler.hpp"

namespace visos {

SyscallResult SyscallHandler::handle(const Syscall& syscall,
                                      [[maybe_unused]] const ProcessControlBlock& pcb) const {
    // All syscalls in the current model are blocking operations
    return SyscallResult{KernelEvent::SyscallBlock, syscall};
}

} // namespace visos
