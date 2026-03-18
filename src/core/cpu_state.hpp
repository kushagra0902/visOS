#pragma once

#include <cstdint>
#include <string_view>

namespace visos {

enum class CPUState : std::uint8_t {
    Idle,
    Executing
};

[[nodiscard]] constexpr std::string_view to_string(CPUState s) noexcept {
    switch (s) {
        case CPUState::Idle:      return "IDLE";
        case CPUState::Executing: return "EXECUTING";
    }
    return "UNKNOWN";
}

} // namespace visos
