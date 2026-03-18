#pragma once

#include "core/types.hpp"

#include <functional>

namespace visos {

class TimerInterrupt {
public:
    using InterruptCallback = std::function<void(Tick)>;

    explicit TimerInterrupt(InterruptCallback on_interrupt);

    void trigger(Tick current_time);

private:
    InterruptCallback on_interrupt_;
};

} // namespace visos
