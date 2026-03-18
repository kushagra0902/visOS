#include "core/process_control_block.hpp"

#include <stdexcept>
#include <string>

namespace visos {

ProcessControlBlock::ProcessControlBlock(PID pid, ResourceDescriptor resources,
                                         Tick estimated_cpu_time)
    : pid_(pid)
    , estimated_cpu_time_(estimated_cpu_time)
    , required_resources_(resources)
{
}

void ProcessControlBlock::applyTransition(StateTransition transition) {
    if (!isValidTransition(state_, transition)) {
        throw std::logic_error(
            "Invalid state transition: cannot apply " +
            std::string(to_string(transition)) +
            " from state " +
            std::string(to_string(state_)) +
            " for process PID=" + std::to_string(pid_)
        );
    }
    state_ = targetState(transition);
}

} // namespace visos
