#pragma once

#include "core/types.hpp"

#include <functional>

namespace visos {

class ClockService {
public:
    struct Config {
        int tick_interval = 1; // logical tick interval
    };

    using TickCallback = std::function<void(Tick)>;

    explicit ClockService(Config config, TickCallback on_tick);

    void tick();
    [[nodiscard]] Tick currentTime() const noexcept { return current_time_; }

private:
    Tick current_time_ = 0;
    Config config_;
    TickCallback on_tick_;
};

} // namespace visos
