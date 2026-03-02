#pragma once

#include "core/process_control_block.hpp"
#include "core/types.hpp"

#include <memory>
#include <vector>
#include <cstddef>

namespace visos {

struct BlockedEntry {
    std::shared_ptr<ProcessControlBlock> pcb;
    Tick wakeup_tick; // absolute tick when I/O completes
};

class BlockedQueue {
public:
    void add(std::shared_ptr<ProcessControlBlock> pcb, Tick wakeup_tick);

    // Returns all processes whose I/O has completed by current_tick
    [[nodiscard]] std::vector<std::shared_ptr<ProcessControlBlock>>
        getCompleted(Tick current_tick);

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    // Read-only access for inspection
    [[nodiscard]] const std::vector<BlockedEntry>& entries() const noexcept;

private:
    std::vector<BlockedEntry> entries_;
};

} // namespace visos
