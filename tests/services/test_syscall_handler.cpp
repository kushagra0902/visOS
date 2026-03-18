#include "services/syscall_handler.hpp"
#include "core/process_control_block.hpp"

#include <gtest/gtest.h>

using namespace visos;

TEST(SyscallHandlerTest, HandleReturnsBlockEvent) {
    SyscallHandler handler;
    ProcessControlBlock pcb(1, ResourceDescriptor{}, 10);

    Syscall syscall{SyscallType::IoRead, 5};
    auto result = handler.handle(syscall, pcb);

    EXPECT_EQ(result.event, KernelEvent::SyscallBlock);
    EXPECT_EQ(result.syscall.type, SyscallType::IoRead);
    EXPECT_EQ(result.syscall.duration, 5);
}

TEST(SyscallHandlerTest, AllSyscallTypesBlock) {
    SyscallHandler handler;
    ProcessControlBlock pcb(1, ResourceDescriptor{}, 10);

    for (auto type : {SyscallType::IoRead, SyscallType::IoWrite,
                      SyscallType::Sleep, SyscallType::Wait}) {
        auto result = handler.handle(Syscall{type, 3}, pcb);
        EXPECT_EQ(result.event, KernelEvent::SyscallBlock);
    }
}
