#pragma once

#include "services/ready_queue.hpp"
#include "services/blocked_queue.hpp"
#include "core/process_control_block.hpp"
#include "core/types.hpp"

#include <memory>
#include <vector>

namespace visos {

class MemoryManager {
public:
    void enqueueReady(std::shared_ptr<ProcessControlBlock> pcb);
    void enqueueReadyByPriority(std::shared_ptr<ProcessControlBlock> pcb);
    [[nodiscard]] std::shared_ptr<ProcessControlBlock> dequeueReady();

    void moveToBlocked(std::shared_ptr<ProcessControlBlock> pcb, Tick wakeup_tick);

    // Check for I/O completions and move processes back to ready queue
    [[nodiscard]] std::vector<std::shared_ptr<ProcessControlBlock>>
        checkBlockedCompletions(Tick current_tick);

    [[nodiscard]] bool readyQueueEmpty() const noexcept;
    [[nodiscard]] bool blockedQueueEmpty() const noexcept;

    [[nodiscard]] const ReadyQueue& readyQueue() const noexcept { return ready_queue_; }
    [[nodiscard]] const BlockedQueue& blockedQueue() const noexcept { return blocked_queue_; }

private:
    ReadyQueue ready_queue_;
    BlockedQueue blocked_queue_;
};

} // namespace visos
