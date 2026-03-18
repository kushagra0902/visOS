#include "services/memory_manager.hpp"
#include "core/process_control_block.hpp"

#include <gtest/gtest.h>

using namespace visos;

class MemoryManagerTest : public ::testing::Test {
protected:
    MemoryManager mm;

    std::shared_ptr<ProcessControlBlock> makePCB(PID pid) {
        return std::make_shared<ProcessControlBlock>(pid, ResourceDescriptor{}, 10);
    }
};

TEST_F(MemoryManagerTest, InitiallyEmpty) {
    EXPECT_TRUE(mm.readyQueueEmpty());
    EXPECT_TRUE(mm.blockedQueueEmpty());
}

TEST_F(MemoryManagerTest, EnqueueAndDequeueReady) {
    auto pcb1 = makePCB(1);
    auto pcb2 = makePCB(2);
    mm.enqueueReady(pcb1);
    mm.enqueueReady(pcb2);

    EXPECT_FALSE(mm.readyQueueEmpty());
    auto out1 = mm.dequeueReady();
    EXPECT_EQ(out1->pid(), 1);
    auto out2 = mm.dequeueReady();
    EXPECT_EQ(out2->pid(), 2);
    EXPECT_TRUE(mm.readyQueueEmpty());
}

TEST_F(MemoryManagerTest, DequeueFromEmptyReturnsNull) {
    EXPECT_EQ(mm.dequeueReady(), nullptr);
}

TEST_F(MemoryManagerTest, MoveToBlockedAndComplete) {
    auto pcb = makePCB(1);
    mm.moveToBlocked(pcb, 10);
    EXPECT_FALSE(mm.blockedQueueEmpty());

    auto completed = mm.checkBlockedCompletions(9);
    EXPECT_TRUE(completed.empty());

    completed = mm.checkBlockedCompletions(10);
    EXPECT_EQ(completed.size(), 1u);
    EXPECT_EQ(completed[0]->pid(), 1);
    EXPECT_TRUE(mm.blockedQueueEmpty());
}

TEST_F(MemoryManagerTest, MultipleBlockedCompletions) {
    mm.moveToBlocked(makePCB(1), 5);
    mm.moveToBlocked(makePCB(2), 10);
    mm.moveToBlocked(makePCB(3), 5);

    auto completed = mm.checkBlockedCompletions(5);
    EXPECT_EQ(completed.size(), 2u);
    EXPECT_FALSE(mm.blockedQueueEmpty());

    completed = mm.checkBlockedCompletions(10);
    EXPECT_EQ(completed.size(), 1u);
    EXPECT_TRUE(mm.blockedQueueEmpty());
}
