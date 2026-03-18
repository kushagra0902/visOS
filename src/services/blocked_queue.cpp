#include "services/blocked_queue.hpp"

#include <algorithm>

namespace visos {

void BlockedQueue::add(std::shared_ptr<ProcessControlBlock> pcb, Tick wakeup_tick) {
    entries_.push_back({std::move(pcb), wakeup_tick});
}

std::vector<std::shared_ptr<ProcessControlBlock>>
BlockedQueue::getCompleted(Tick current_tick) {
    std::vector<std::shared_ptr<ProcessControlBlock>> completed;

    auto it = std::partition(entries_.begin(), entries_.end(),
        [current_tick](const BlockedEntry& entry) {
            return entry.wakeup_tick > current_tick;
        });

    for (auto i = it; i != entries_.end(); ++i) {
        completed.push_back(std::move(i->pcb));
    }

    entries_.erase(it, entries_.end());
    return completed;
}

bool BlockedQueue::empty() const noexcept {
    return entries_.empty();
}

std::size_t BlockedQueue::size() const noexcept {
    return entries_.size();
}

const std::vector<BlockedEntry>& BlockedQueue::entries() const noexcept {
    return entries_;
}

} // namespace visos
