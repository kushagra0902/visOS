#include "services/timer_interrupt.hpp"

namespace visos {

TimerInterrupt::TimerInterrupt(InterruptCallback on_interrupt)
    : on_interrupt_(std::move(on_interrupt))
{
}

void TimerInterrupt::trigger(Tick current_time) {
    on_interrupt_(current_time);
}

} // namespace visos
