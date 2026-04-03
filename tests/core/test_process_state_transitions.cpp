#include "core/process_control_block.hpp"
#include "core/resource_descriptor.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

using namespace visos;

class StateTransitionTest : public ::testing::Test {
protected:
    std::shared_ptr<ProcessControlBlock> makePCB() {
        return std::make_shared<ProcessControlBlock>(1, ResourceDescriptor{64, 0}, 10);
    }
};

// --- Invalid transitions from NEW ---
TEST_F(StateTransitionTest, NewCannotDispatch) {
    auto pcb = makePCB();
    EXPECT_THROW(pcb->applyTransition(StateTransition::Dispatch), std::logic_error);
}

TEST_F(StateTransitionTest, NewCannotExit) {
    auto pcb = makePCB();
    EXPECT_THROW(pcb->applyTransition(StateTransition::Exit), std::logic_error);
}

TEST_F(StateTransitionTest, NewCannotBlock) {
    auto pcb = makePCB();
    EXPECT_THROW(pcb->applyTransition(StateTransition::BlockingSyscall), std::logic_error);
}

TEST_F(StateTransitionTest, NewCannotQuantumExpire) {
    auto pcb = makePCB();
    EXPECT_THROW(pcb->applyTransition(StateTransition::QuantumExpired), std::logic_error);
}

TEST_F(StateTransitionTest, NewCannotIoComplete) {
    auto pcb = makePCB();
    EXPECT_THROW(pcb->applyTransition(StateTransition::IoComplete), std::logic_error);
}

// --- Invalid transitions from READY ---
TEST_F(StateTransitionTest, ReadyCannotAdmit) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Admit), std::logic_error);
}

TEST_F(StateTransitionTest, ReadyCannotExit) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Exit), std::logic_error);
}

TEST_F(StateTransitionTest, ReadyCannotBlock) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    EXPECT_THROW(pcb->applyTransition(StateTransition::BlockingSyscall), std::logic_error);
}

// --- Invalid transitions from RUNNING ---
TEST_F(StateTransitionTest, RunningCannotAdmit) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Admit), std::logic_error);
}

TEST_F(StateTransitionTest, RunningCannotDispatch) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Dispatch), std::logic_error);
}

TEST_F(StateTransitionTest, RunningCannotIoComplete) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    EXPECT_THROW(pcb->applyTransition(StateTransition::IoComplete), std::logic_error);
}

// --- Invalid transitions from BLOCKED ---
TEST_F(StateTransitionTest, BlockedCannotDispatch) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::BlockingSyscall);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Dispatch), std::logic_error);
}

TEST_F(StateTransitionTest, BlockedCannotExit) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::BlockingSyscall);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Exit), std::logic_error);
}

// --- Invalid transitions from TERMINATED ---
TEST_F(StateTransitionTest, TerminatedCannotTransition) {
    auto pcb = makePCB();
    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::Exit);

    EXPECT_THROW(pcb->applyTransition(StateTransition::Admit), std::logic_error);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Dispatch), std::logic_error);
    EXPECT_THROW(pcb->applyTransition(StateTransition::QuantumExpired), std::logic_error);
    EXPECT_THROW(pcb->applyTransition(StateTransition::BlockingSyscall), std::logic_error);
    EXPECT_THROW(pcb->applyTransition(StateTransition::IoComplete), std::logic_error);
    EXPECT_THROW(pcb->applyTransition(StateTransition::Exit), std::logic_error);
}

// --- Full lifecycle test ---
TEST_F(StateTransitionTest, FullLifecycleWithBlocking) {
    auto pcb = makePCB();

    EXPECT_EQ(pcb->state(), ProcessState::New);
    pcb->applyTransition(StateTransition::Admit);
    EXPECT_EQ(pcb->state(), ProcessState::Ready);
    pcb->applyTransition(StateTransition::Dispatch);
    EXPECT_EQ(pcb->state(), ProcessState::Running);
    pcb->applyTransition(StateTransition::BlockingSyscall);
    EXPECT_EQ(pcb->state(), ProcessState::Blocked);
    pcb->applyTransition(StateTransition::IoComplete);
    EXPECT_EQ(pcb->state(), ProcessState::Ready);
    pcb->applyTransition(StateTransition::Dispatch);
    EXPECT_EQ(pcb->state(), ProcessState::Running);
    pcb->applyTransition(StateTransition::Exit);
    EXPECT_EQ(pcb->state(), ProcessState::Terminated);
}

TEST_F(StateTransitionTest, FullLifecycleWithPreemption) {
    auto pcb = makePCB();

    pcb->applyTransition(StateTransition::Admit);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::QuantumExpired);
    EXPECT_EQ(pcb->state(), ProcessState::Ready);
    pcb->applyTransition(StateTransition::Dispatch);
    pcb->applyTransition(StateTransition::Exit);
    EXPECT_EQ(pcb->state(), ProcessState::Terminated);
}
