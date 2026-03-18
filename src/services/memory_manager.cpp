#include "services/memory_manager.hpp"

namespace visos {

void MemoryManager::enqueueReady(std::shared_ptr<ProcessControlBlock> pcb) {
    ready_queue_.enqueue(std::move(pcb));
}

void MemoryManager::enqueueReadyByPriority(std::shared_ptr<ProcessControlBlock> pcb) {
    ready_queue_.enqueueByPriority(std::move(pcb));
}

std::shared_ptr<ProcessControlBlock> MemoryManager::dequeueReady() {
    return ready_queue_.dequeue();
}

void MemoryManager::moveToBlocked(std::shared_ptr<ProcessControlBlock> pcb,
                                   Tick wakeup_tick) {
    blocked_queue_.add(std::move(pcb), wakeup_tick);
}

std::vector<std::shared_ptr<ProcessControlBlock>>
MemoryManager::checkBlockedCompletions(Tick current_tick) {
    return blocked_queue_.getCompleted(current_tick);
}

bool MemoryManager::readyQueueEmpty() const noexcept {
    return ready_queue_.empty();
}

bool MemoryManager::blockedQueueEmpty() const noexcept {
    return blocked_queue_.empty();
}

} // namespace visos
