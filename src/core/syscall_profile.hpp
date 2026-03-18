#pragma once

#include "core/syscall.hpp"
#include "core/types.hpp"

#include <map>
#include <optional>

namespace visos {

// Maps a relative CPU tick (within the process execution) to a syscall.
// This ensures deterministic simulation: we know exactly when each syscall fires.
class SyscallProfile {
public:
    SyscallProfile() = default;

    void addSyscall(Tick at_tick, Syscall syscall) {
        syscalls_[at_tick] = syscall;
    }

    [[nodiscard]] std::optional<Syscall> getSyscallAt(Tick tick) const {
        auto it = syscalls_.find(tick);
        if (it != syscalls_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool empty() const noexcept { return syscalls_.empty(); }

private:
    std::map<Tick, Syscall> syscalls_;
};

} // namespace visos
