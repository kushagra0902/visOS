#include "kernel/simulation_engine.hpp"
#include "util/logger.hpp"

namespace visos {

SimulationEngine::SimulationEngine(Kernel& kernel, Config config)
    : kernel_(kernel)
    , config_(config)
{
    ClockService::Config clock_config{config_.tick_interval};
    clock_ = std::make_unique<ClockService>(clock_config,
        [this](Tick tick) {
            tick_count_ = tick;
            kernel_.notifyTick(tick);
        });
}

void SimulationEngine::run() {
    running_ = true;
    Logger::logMessage("Simulation started.");

    while (running_ && kernel_.hasActiveProcesses() &&
           tick_count_ < config_.max_ticks) {
        clock_->tick();
    }

    running_ = false;

    if (tick_count_ >= config_.max_ticks) {
        Logger::logMessage("Simulation stopped: max ticks reached.");
    } else {
        Logger::logMessage("Simulation completed.");
    }
}

void SimulationEngine::step() {
    if (kernel_.hasActiveProcesses()) {
        clock_->tick();
    }
}

void SimulationEngine::stop() noexcept {
    running_ = false;
}

bool SimulationEngine::isRunning() const noexcept {
    return running_;
}

Tick SimulationEngine::currentTick() const noexcept {
    return tick_count_;
}

} // namespace visos
