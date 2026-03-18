#pragma once

#include "core/types.hpp"

namespace visos {

class PidGenerator {
public:
    [[nodiscard]] PID next() noexcept { return next_pid_++; }
    void reset() noexcept { next_pid_ = 1; }

private:
    PID next_pid_ = 1;
};

} // namespace visos
