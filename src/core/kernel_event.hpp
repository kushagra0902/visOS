#pragma once

#include <cstdint>
#include <string_view>

namespace visos {

enum class KernelEvent : std::uint8_t {
    QuantumExpired,
    ProcessExit,
    SyscallBlock,
    CpuIdle
};

[[nodiscard]] constexpr std::string_view to_string(KernelEvent e) noexcept {
    switch (e) {
        case KernelEvent::QuantumExpired: return "QUANTUM_EXPIRED";
        case KernelEvent::ProcessExit:    return "PROCESS_EXIT";
        case KernelEvent::SyscallBlock:   return "SYSCALL_BLOCK";
        case KernelEvent::CpuIdle:        return "CPU_IDLE";
    }
    return "UNKNOWN";
}

} // namespace visos
