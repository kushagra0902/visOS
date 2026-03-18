#include "services/clock_service.hpp"

namespace visos {

ClockService::ClockService(Config config, TickCallback on_tick)
    : config_(config)
    , on_tick_(std::move(on_tick))
{
}

void ClockService::tick() {
    current_time_ += config_.tick_interval;
    on_tick_(current_time_);
}

} // namespace visos
