#include "services/cpu.hpp"
#include "core/process_control_block.hpp"
#include "core/kernel_event.hpp"
#include "core/syscall_profile.hpp"

#include <gtest/gtest.h>
#include <optional>

using namespace visos;

class CPUTest : public ::testing::Test {
protected:
    std::optional<KernelEvent> last_event_;
    ProcessControlBlock* last_pcb_ = nullptr;

    std::unique_ptr<CPU> makeCPU() {
        return std::make_unique<CPU>(
            [this](KernelEvent event, ProcessControlBlock* pcb) {
                last_event_ = event;
                last_pcb_ = pcb;
            });
    }

    std::shared_ptr<ProcessControlBlock> makePCB(Tick cpu_time = 10) {
        auto pcb = std::make_shared<ProcessControlBlock>(1, ResourceDescriptor{}, cpu_time);
        pcb->applyTransition(StateTransition::Admit);
        pcb->applyTransition(StateTransition::Dispatch);
        return pcb;
    }
};

TEST_F(CPUTest, InitialStateIsIdle) {
    auto cpu = makeCPU();
    EXPECT_EQ(cpu->state(), CPUState::Idle);
    EXPECT_EQ(cpu->currentProcess(), nullptr);
}

TEST_F(CPUTest, IdleCPURaisesCpuIdleEvent) {
    auto cpu = makeCPU();
    cpu->executeTick();
    ASSERT_TRUE(last_event_.has_value());
    EXPECT_EQ(last_event_.value(), KernelEvent::CpuIdle);
}

TEST_F(CPUTest, LoadProcessChangesState) {
    auto cpu = makeCPU();
    auto pcb = makePCB();
    cpu->loadProcess(pcb, SyscallProfile{}, 4);
    EXPECT_EQ(cpu->state(), CPUState::Executing);
    EXPECT_EQ(cpu->currentProcess()->pid(), 1);
    EXPECT_EQ(cpu->remainingQuantum(), 4);
}

TEST_F(CPUTest, UnloadProcessReturnsToIdle) {
    auto cpu = makeCPU();
    auto pcb = makePCB();
    cpu->loadProcess(pcb, SyscallProfile{}, 4);
    auto unloaded = cpu->unloadProcess();
    EXPECT_EQ(cpu->state(), CPUState::Idle);
    EXPECT_EQ(unloaded->pid(), 1);
}

TEST_F(CPUTest, ExecuteTickIncrementsCpuTime) {
    auto cpu = makeCPU();
    auto pcb = makePCB();
    cpu->loadProcess(pcb, SyscallProfile{}, 10);
    cpu->executeTick();
    EXPECT_EQ(pcb->cpuTimeUsed(), 1);
}

TEST_F(CPUTest, QuantumExpiresAfterTicks) {
    auto cpu = makeCPU();
    auto pcb = makePCB(10);
    cpu->loadProcess(pcb, SyscallProfile{}, 2);

    cpu->executeTick(); // quantum = 1
    last_event_.reset();
    cpu->executeTick(); // quantum = 0 -> QUANTUM_EXPIRED

    ASSERT_TRUE(last_event_.has_value());
    EXPECT_EQ(last_event_.value(), KernelEvent::QuantumExpired);
}

TEST_F(CPUTest, ProcessExitWhenFinished) {
    auto cpu = makeCPU();
    auto pcb = makePCB(2);
    cpu->loadProcess(pcb, SyscallProfile{}, 10);

    cpu->executeTick(); // cpu_time = 1
    last_event_.reset();
    cpu->executeTick(); // cpu_time = 2 -> finished

    ASSERT_TRUE(last_event_.has_value());
    EXPECT_EQ(last_event_.value(), KernelEvent::ProcessExit);
}

TEST_F(CPUTest, SyscallBlockAtCorrectTick) {
    auto cpu = makeCPU();
    auto pcb = makePCB(10);

    SyscallProfile profile;
    profile.addSyscall(2, Syscall{SyscallType::IoRead, 3});

    cpu->loadProcess(pcb, profile, 10);
    cpu->executeTick(); // cpu_time = 1, no syscall
    last_event_.reset();
    cpu->executeTick(); // cpu_time = 2, syscall fires

    ASSERT_TRUE(last_event_.has_value());
    EXPECT_EQ(last_event_.value(), KernelEvent::SyscallBlock);
}
