#include "services/ready_queue.hpp"

#include <algorithm>

namespace visos {

void ReadyQueue::enqueue(std::shared_ptr<ProcessControlBlock> pcb) {
    queue_.push_back(std::move(pcb));
}

std::shared_ptr<ProcessControlBlock> ReadyQueue::dequeue() {
    if (queue_.empty()) {
        return nullptr;
    }
    auto pcb = std::move(queue_.front());
    queue_.pop_front();
    return pcb;
}

bool ReadyQueue::empty() const noexcept {
    return queue_.empty();
}

std::size_t ReadyQueue::size() const noexcept {
    return queue_.size();
}

void ReadyQueue::enqueueByPriority(std::shared_ptr<ProcessControlBlock> pcb) {
    auto it = std::find_if(queue_.begin(), queue_.end(),
        [&pcb](const std::shared_ptr<ProcessControlBlock>& existing) {
            return existing->priority() < pcb->priority();
        });
    queue_.insert(it, std::move(pcb));
}

const std::deque<std::shared_ptr<ProcessControlBlock>>& ReadyQueue::entries() const noexcept {
    return queue_;
}

} // namespace visos
