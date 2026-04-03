#pragma once

#include "kernel/kernel.hpp"
#include "services/clock_service.hpp"

#include <atomic>

namespace visos {

class SimulationEngine {
public:
    struct Config {
        int tick_interval;
        int max_ticks;

        Config() : tick_interval(1), max_ticks(1000) {}
        Config(int interval, int max) : tick_interval(interval), max_ticks(max) {}
    };

    explicit SimulationEngine(Kernel& kernel, Config config = Config{});

    // Run simulation until all processes terminate or max_ticks reached
    void run();

    // Execute a single tick (for step-by-step mode)
    void step();

    void stop() noexcept;
    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] Tick currentTick() const noexcept;

private:
    Kernel& kernel_;
    std::unique_ptr<ClockService> clock_;
    Config config_;
    std::atomic<bool> running_{false};
    Tick tick_count_ = 0;
};

} // namespace visos
