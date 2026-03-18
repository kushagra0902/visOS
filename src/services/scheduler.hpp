#pragma once

#include "core/scheduling_policy.hpp"
#include "core/process_control_block.hpp"
#include "services/memory_manager.hpp"
#include "services/scheduling/scheduling_strategy.hpp"

#include <memory>

namespace visos {

class Scheduler {
public:
    Scheduler(SchedulingPolicy policy, int time_quantum,
              MemoryManager& memory_manager);

    [[nodiscard]] std::shared_ptr<ProcessControlBlock> selectNextProcess();

    void onProcessBlocked(std::shared_ptr<ProcessControlBlock> pcb);
    void onProcessExit(std::shared_ptr<ProcessControlBlock> pcb);

    [[nodiscard]] SchedulingPolicy policy() const noexcept { return policy_; }
    [[nodiscard]] int defaultQuantum() const noexcept;

private:
    SchedulingPolicy policy_;
    std::unique_ptr<ISchedulingStrategy> strategy_;
    MemoryManager& memory_manager_;

    static std::unique_ptr<ISchedulingStrategy>
        createStrategy(SchedulingPolicy policy, int time_quantum);
};

} // namespace visos
