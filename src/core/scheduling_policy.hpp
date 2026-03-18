#pragma once

#include <cstdint>
#include <string_view>

namespace visos {

enum class SchedulingPolicy : std::uint8_t {
    FCFS,
    RoundRobin,
    Priority
};

[[nodiscard]] constexpr std::string_view to_string(SchedulingPolicy p) noexcept {
    switch (p) {
        case SchedulingPolicy::FCFS:       return "FCFS";
        case SchedulingPolicy::RoundRobin: return "ROUND_ROBIN";
        case SchedulingPolicy::Priority:   return "PRIORITY";
    }
    return "UNKNOWN";
}

} // namespace visos
