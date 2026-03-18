#include "services/scheduler.hpp"
#include "services/scheduling/fcfs_strategy.hpp"
#include "services/scheduling/round_robin_strategy.hpp"
#include "services/scheduling/priority_strategy.hpp"

#include <stdexcept>

namespace visos {

Scheduler::Scheduler(SchedulingPolicy policy, int time_quantum,
                     MemoryManager& memory_manager)
    : policy_(policy)
    , strategy_(createStrategy(policy, time_quantum))
    , memory_manager_(memory_manager)
{
}

std::shared_ptr<ProcessControlBlock> Scheduler::selectNextProcess() {
    // Access the underlying deque through the ready queue for strategy selection
    // The strategy selects and removes from the queue
    auto& ready_entries = const_cast<std::deque<std::shared_ptr<ProcessControlBlock>>&>(
        memory_manager_.readyQueue().entries());
    return strategy_->selectNext(ready_entries);
}

void Scheduler::onProcessBlocked(
    [[maybe_unused]] std::shared_ptr<ProcessControlBlock> pcb) {
    // Process has already been moved to blocked queue by the Kernel.
    // The Scheduler is notified for logging/metrics purposes.
}

void Scheduler::onProcessExit(
    [[maybe_unused]] std::shared_ptr<ProcessControlBlock> pcb) {
    // Process has been terminated. Notification for logging/metrics.
}

int Scheduler::defaultQuantum() const noexcept {
    return strategy_->defaultQuantum();
}

std::unique_ptr<ISchedulingStrategy>
Scheduler::createStrategy(SchedulingPolicy policy, int time_quantum) {
    switch (policy) {
        case SchedulingPolicy::FCFS:
            return std::make_unique<FCFSStrategy>();
        case SchedulingPolicy::RoundRobin:
            return std::make_unique<RoundRobinStrategy>(time_quantum);
        case SchedulingPolicy::Priority:
            return std::make_unique<PriorityStrategy>(time_quantum);
    }
    throw std::logic_error("Unknown scheduling policy");
}

} // namespace visos
