#pragma once

#include "core/process_control_block.hpp"

#include <deque>
#include <memory>

namespace visos {

class ISchedulingStrategy {
public:
    virtual ~ISchedulingStrategy() = default;

    // Select and remove the next process from the ready queue.
    // Returns nullptr if no process is available.
    [[nodiscard]] virtual std::shared_ptr<ProcessControlBlock>
        selectNext(std::deque<std::shared_ptr<ProcessControlBlock>>& ready_queue) = 0;

    // Returns the default time quantum for this policy
    [[nodiscard]] virtual int defaultQuantum() const noexcept = 0;
};

} // namespace visos
