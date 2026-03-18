#include "services/scheduling/round_robin_strategy.hpp"

namespace visos {

RoundRobinStrategy::RoundRobinStrategy(int time_quantum)
    : time_quantum_(time_quantum)
{
}

std::shared_ptr<ProcessControlBlock>
RoundRobinStrategy::selectNext(
    std::deque<std::shared_ptr<ProcessControlBlock>>& ready_queue) {
    if (ready_queue.empty()) {
        return nullptr;
    }
    auto pcb = std::move(ready_queue.front());
    ready_queue.pop_front();
    return pcb;
}

int RoundRobinStrategy::defaultQuantum() const noexcept {
    return time_quantum_;
}

} // namespace visos
