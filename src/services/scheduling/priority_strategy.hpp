#pragma once

#include "services/scheduling/scheduling_strategy.hpp"

namespace visos {

class PriorityStrategy : public ISchedulingStrategy {
public:
    explicit PriorityStrategy(int time_quantum = 4);

    [[nodiscard]] std::shared_ptr<ProcessControlBlock>
        selectNext(std::deque<std::shared_ptr<ProcessControlBlock>>& ready_queue) override;

    [[nodiscard]] int defaultQuantum() const noexcept override;

private:
    int time_quantum_;
};

} // namespace visos
