#include "services/scheduling/fcfs_strategy.hpp"
#include "services/scheduling/round_robin_strategy.hpp"
#include "services/scheduling/priority_strategy.hpp"
#include "core/process_control_block.hpp"

#include <gtest/gtest.h>
#include <deque>
#include <memory>

using namespace visos;

class StrategyTest : public ::testing::Test {
protected:
    std::deque<std::shared_ptr<ProcessControlBlock>> queue;

    std::shared_ptr<ProcessControlBlock> makePCB(PID pid, int priority = 0) {
        auto pcb = std::make_shared<ProcessControlBlock>(pid, ResourceDescriptor{}, 10);
        pcb->setPriority(priority);
        return pcb;
    }
};

TEST_F(StrategyTest, FCFSSelectsFront) {
    FCFSStrategy strategy;
    queue.push_back(makePCB(1));
    queue.push_back(makePCB(2));
    queue.push_back(makePCB(3));

    auto selected = strategy.selectNext(queue);
    EXPECT_EQ(selected->pid(), 1);
    EXPECT_EQ(queue.size(), 2u);
}

TEST_F(StrategyTest, FCFSEmptyQueue) {
    FCFSStrategy strategy;
    EXPECT_EQ(strategy.selectNext(queue), nullptr);
}

TEST_F(StrategyTest, RoundRobinSelectsFront) {
    RoundRobinStrategy strategy(3);
    queue.push_back(makePCB(1));
    queue.push_back(makePCB(2));

    auto selected = strategy.selectNext(queue);
    EXPECT_EQ(selected->pid(), 1);
    EXPECT_EQ(strategy.defaultQuantum(), 3);
}

TEST_F(StrategyTest, PrioritySelectsHighest) {
    PriorityStrategy strategy(4);
    queue.push_back(makePCB(1, 3));
    queue.push_back(makePCB(2, 7));
    queue.push_back(makePCB(3, 1));

    auto selected = strategy.selectNext(queue);
    EXPECT_EQ(selected->pid(), 2);
    EXPECT_EQ(queue.size(), 2u);
}

TEST_F(StrategyTest, PriorityTieBreaksStably) {
    PriorityStrategy strategy(4);
    queue.push_back(makePCB(1, 5));
    queue.push_back(makePCB(2, 5));

    // std::max_element returns the first maximum found
    auto selected = strategy.selectNext(queue);
    EXPECT_EQ(selected->pid(), 1);
}
