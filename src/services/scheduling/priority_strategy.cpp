#include "services/scheduling/priority_strategy.hpp"

#include <algorithm>

namespace visos {

PriorityStrategy::PriorityStrategy(int time_quantum)
    : time_quantum_(time_quantum)
{
}

std::shared_ptr<ProcessControlBlock>
PriorityStrategy::selectNext(
    std::deque<std::shared_ptr<ProcessControlBlock>>& ready_queue) {
    if (ready_queue.empty()) {
        return nullptr;
    }

    // Find the process with the highest priority
    auto best = std::max_element(ready_queue.begin(), ready_queue.end(),
        [](const std::shared_ptr<ProcessControlBlock>& a,
           const std::shared_ptr<ProcessControlBlock>& b) {
            return a->priority() < b->priority();
        });

    auto pcb = std::move(*best);
    ready_queue.erase(best);
    return pcb;
}

int PriorityStrategy::defaultQuantum() const noexcept {
    return time_quantum_;
}

} // namespace visos
