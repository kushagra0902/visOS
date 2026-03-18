#include "services/scheduler.hpp"
#include "services/memory_manager.hpp"
#include "core/process_control_block.hpp"
#include "core/scheduling_policy.hpp"

#include <gtest/gtest.h>

using namespace visos;

class SchedulerTest : public ::testing::Test {
protected:
    MemoryManager mm;

    std::shared_ptr<ProcessControlBlock> makePCB(PID pid, int priority = 0) {
        auto pcb = std::make_shared<ProcessControlBlock>(pid, ResourceDescriptor{}, 10);
        pcb->setPriority(priority);
        return pcb;
    }
};

TEST_F(SchedulerTest, FCFSSelectsInOrder) {
    Scheduler sched(SchedulingPolicy::FCFS, 0, mm);
    mm.enqueueReady(makePCB(1));
    mm.enqueueReady(makePCB(2));
    mm.enqueueReady(makePCB(3));

    auto p = sched.selectNextProcess();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->pid(), 1);

    p = sched.selectNextProcess();
    EXPECT_EQ(p->pid(), 2);

    p = sched.selectNextProcess();
    EXPECT_EQ(p->pid(), 3);

    p = sched.selectNextProcess();
    EXPECT_EQ(p, nullptr);
}

TEST_F(SchedulerTest, RoundRobinSelectsInOrder) {
    Scheduler sched(SchedulingPolicy::RoundRobin, 4, mm);
    mm.enqueueReady(makePCB(1));
    mm.enqueueReady(makePCB(2));

    auto p = sched.selectNextProcess();
    EXPECT_EQ(p->pid(), 1);
    EXPECT_EQ(sched.defaultQuantum(), 4);
}

TEST_F(SchedulerTest, PrioritySelectsHighestFirst) {
    Scheduler sched(SchedulingPolicy::Priority, 4, mm);
    mm.enqueueReady(makePCB(1, 5));
    mm.enqueueReady(makePCB(2, 10));
    mm.enqueueReady(makePCB(3, 1));

    auto p = sched.selectNextProcess();
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->pid(), 2); // highest priority

    p = sched.selectNextProcess();
    EXPECT_EQ(p->pid(), 1);

    p = sched.selectNextProcess();
    EXPECT_EQ(p->pid(), 3);
}

TEST_F(SchedulerTest, EmptyQueueReturnsNull) {
    Scheduler sched(SchedulingPolicy::FCFS, 0, mm);
    EXPECT_EQ(sched.selectNextProcess(), nullptr);
}
