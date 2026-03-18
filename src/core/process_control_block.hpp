#pragma once

#include "core/types.hpp"
#include "core/process_state.hpp"
#include "core/register_set.hpp"
#include "core/resource_descriptor.hpp"

#include <cstdint>
#include <string_view>

namespace visos {

// Transition triggers matching the UML state machine diagram exactly
enum class StateTransition : std::uint8_t {
    Admit,            // NEW -> READY
    Dispatch,         // READY -> RUNNING
    QuantumExpired,   // RUNNING -> READY
    BlockingSyscall,  // RUNNING -> BLOCKED
    IoComplete,       // BLOCKED -> READY
    Exit              // RUNNING -> TERMINATED
};

[[nodiscard]] constexpr std::string_view to_string(StateTransition t) noexcept {
    switch (t) {
        case StateTransition::Admit:           return "ADMIT";
        case StateTransition::Dispatch:        return "DISPATCH";
        case StateTransition::QuantumExpired:  return "QUANTUM_EXPIRED";
        case StateTransition::BlockingSyscall: return "BLOCKING_SYSCALL";
        case StateTransition::IoComplete:      return "IO_COMPLETE";
        case StateTransition::Exit:            return "EXIT";
    }
    return "UNKNOWN";
}

class ProcessControlBlock {
public:
    ProcessControlBlock(PID pid, ResourceDescriptor resources,
                        Tick estimated_cpu_time);

    // The ONLY way to change state. Throws std::logic_error on illegal transition.
    void applyTransition(StateTransition transition);

    [[nodiscard]] PID pid() const noexcept { return pid_; }
    [[nodiscard]] ProcessState state() const noexcept { return state_; }
    [[nodiscard]] int priority() const noexcept { return priority_; }
    [[nodiscard]] Tick cpuTimeUsed() const noexcept { return cpu_time_used_; }
    [[nodiscard]] Tick estimatedCpuTime() const noexcept { return estimated_cpu_time_; }
    [[nodiscard]] const RegisterSet& registers() const noexcept { return registers_; }
    [[nodiscard]] const ResourceDescriptor& requiredResources() const noexcept {
        return required_resources_;
    }

    void incrementCpuTime(Tick ticks) noexcept { cpu_time_used_ += ticks; }
    void setRegisters(RegisterSet regs) noexcept { registers_ = regs; }
    void setPriority(int p) noexcept { priority_ = p; }

    // Returns true if process has used all its estimated CPU time
    [[nodiscard]] bool isFinished() const noexcept {
        return cpu_time_used_ >= estimated_cpu_time_;
    }

private:
    [[nodiscard]] static constexpr bool isValidTransition(
        ProcessState from, StateTransition t) noexcept;

    [[nodiscard]] static constexpr ProcessState targetState(
        StateTransition t) noexcept;

    PID pid_;
    ProcessState state_ = ProcessState::New;
    int priority_ = 0;
    Tick cpu_time_used_ = 0;
    Tick estimated_cpu_time_;
    RegisterSet registers_{};
    ResourceDescriptor required_resources_;
};

constexpr bool ProcessControlBlock::isValidTransition(
    ProcessState from, StateTransition t) noexcept
{
    switch (t) {
        case StateTransition::Admit:           return from == ProcessState::New;
        case StateTransition::Dispatch:        return from == ProcessState::Ready;
        case StateTransition::QuantumExpired:  return from == ProcessState::Running;
        case StateTransition::BlockingSyscall: return from == ProcessState::Running;
        case StateTransition::IoComplete:      return from == ProcessState::Blocked;
        case StateTransition::Exit:            return from == ProcessState::Running;
    }
    return false;
}

constexpr ProcessState ProcessControlBlock::targetState(
    StateTransition t) noexcept
{
    switch (t) {
        case StateTransition::Admit:           return ProcessState::Ready;
        case StateTransition::Dispatch:        return ProcessState::Running;
        case StateTransition::QuantumExpired:  return ProcessState::Ready;
        case StateTransition::BlockingSyscall: return ProcessState::Blocked;
        case StateTransition::IoComplete:      return ProcessState::Ready;
        case StateTransition::Exit:            return ProcessState::Terminated;
    }
    return ProcessState::Terminated;
}

} // namespace visos
