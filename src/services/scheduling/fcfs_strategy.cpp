#include "services/scheduling/fcfs_strategy.hpp"

#include <limits>

namespace visos {

std::shared_ptr<ProcessControlBlock>
FCFSStrategy::selectNext(std::deque<std::shared_ptr<ProcessControlBlock>>& ready_queue) {
    if (ready_queue.empty()) {
        return nullptr;
    }
    auto pcb = std::move(ready_queue.front());
    ready_queue.pop_front();
    return pcb;
}

int FCFSStrategy::defaultQuantum() const noexcept {
    // FCFS: no preemption, effectively infinite quantum
    return std::numeric_limits<int>::max();
}

} // namespace visos
