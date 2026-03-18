#include "core/process_control_block.hpp"
#include "core/resource_descriptor.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

using namespace visos;

class PCBTest : public ::testing::Test {
protected:
    std::shared_ptr<ProcessControlBlock> makePCB(PID pid = 1, Tick cpu_time = 10) {
        return std::make_shared<ProcessControlBlock>(pid, ResourceDescriptor{64, 0}, cpu_time);
    }
};

TEST_F(PCBTest, InitialStateIsNew) {
    auto pcb = makePCB();
    EXPECT_EQ(pcb->state(), ProcessState::New);
    EXPECT_EQ(pcb->pid(), 1);
    EXPECT_EQ(pcb->cpuTimeUsed(), 0);
    EXPECT_EQ(pcb->priority(), 0);
}

TEST_F(PCBTest, AdmitTransitionsNewToReady) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    EXPECT_EQ(pcb->state(), ProcessState::Ready);
}

TEST_F(PCBTest, DispatchTransitionsReadyToRunning) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    EXPECT_EQ(pcb->state(), ProcessState::Running);
}

TEST_F(PCBTest, ExitTransitionsRunningToTerminated) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::Exit);
    EXPECT_EQ(pcb->state(), ProcessState::Terminated);
}

TEST_F(PCBTest, QuantumExpiredTransitionsRunningToReady) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::QuantumExpired);
    EXPECT_EQ(pcb->state(), ProcessState::Ready);
}

TEST_F(PCBTest, BlockingSyscallTransitionsRunningToBlocked) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::BlockingSyscall);
    EXPECT_EQ(pcb->state(), ProcessState::Blocked);
}

TEST_F(PCBTest, IoCompleteTransitionsBlockedToReady) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::BlockingSyscall);
    pcb->applyTransition(StateTransition::IoComplete);
    EXPECT_EQ(pcb->state(), ProcessState::Ready);
}

TEST_F(PCBTest, IncrementCpuTime) {
    auto pcb = makePCB();
    pcb->incrementCpuTime(5);
    EXPECT_EQ(pcb->cpuTimeUsed(), 5);
    pcb->incrementCpuTime(3);
    EXPECT_EQ(pcb->cpuTimeUsed(), 8);
}

TEST_F(PCBTest, IsFinished) {
    auto pcb = makePCB(1, 5);
    EXPECT_FALSE(pcb->isFinished());
    pcb->incrementCpuTime(5);
    EXPECT_TRUE(pcb->isFinished());
}

TEST_F(PCBTest, SetPriority) {
    auto pcb = makePCB();
    pcb->setPriority(10);
    EXPECT_EQ(pcb->priority(), 10);
}

TEST_F(PCBTest, SetRegisters) {
    auto pcb = makePCB();
    RegisterSet regs;
    regs.program_counter = 42;
    pcb->setRegisters(regs);
    EXPECT_EQ(pcb->registers().program_counter, 42);
}
