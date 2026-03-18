#pragma once

#include "core/process_control_block.hpp"

#include <deque>
#include <memory>
#include <cstddef>

namespace visos {

class ReadyQueue {
public:
    void enqueue(std::shared_ptr<ProcessControlBlock> pcb);
    [[nodiscard]] std::shared_ptr<ProcessControlBlock> dequeue();
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    // For priority scheduling: insert in priority order
    void enqueueByPriority(std::shared_ptr<ProcessControlBlock> pcb);

    // Read-only access for inspection
    [[nodiscard]] const std::deque<std::shared_ptr<ProcessControlBlock>>& entries() const noexcept;

private:
    std::deque<std::shared_ptr<ProcessControlBlock>> queue_;
};

} // namespace visos
