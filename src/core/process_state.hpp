#pragma once

#include <cstdint>
#include <string_view>

namespace visos {

enum class ProcessState : std::uint8_t {
    New,
    Ready,
    Running,
    Blocked,
    Terminated
};

[[nodiscard]] constexpr std::string_view to_string(ProcessState s) noexcept {
    switch (s) {
        case ProcessState::New:        return "NEW";
        case ProcessState::Ready:      return "READY";
        case ProcessState::Running:    return "RUNNING";
        case ProcessState::Blocked:    return "BLOCKED";
        case ProcessState::Terminated: return "TERMINATED";
    }
    return "UNKNOWN";
}

} // namespace visos
