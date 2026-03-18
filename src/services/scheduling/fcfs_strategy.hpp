#pragma once

#include "services/scheduling/scheduling_strategy.hpp"

namespace visos {

class FCFSStrategy : public ISchedulingStrategy {
public:
    [[nodiscard]] std::shared_ptr<ProcessControlBlock>
        selectNext(std::deque<std::shared_ptr<ProcessControlBlock>>& ready_queue) override;

    // FCFS uses a very large quantum (effectively no preemption)
    [[nodiscard]] int defaultQuantum() const noexcept override;
};

} // namespace visos
