#pragma once

#include <cstdint>
#include <string_view>

namespace visos {

enum class SyscallType : std::uint8_t {
    IoRead,
    IoWrite,
    Sleep,
    Wait
};

[[nodiscard]] constexpr std::string_view to_string(SyscallType t) noexcept {
    switch (t) {
        case SyscallType::IoRead:  return "IO_READ";
        case SyscallType::IoWrite: return "IO_WRITE";
        case SyscallType::Sleep:   return "SLEEP";
        case SyscallType::Wait:    return "WAIT";
    }
    return "UNKNOWN";
}

struct Syscall {
    SyscallType type;
    std::int32_t duration; // ticks until I/O completion

    bool operator==(const Syscall&) const = default;
};

} // namespace visos
