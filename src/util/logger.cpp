#include "util/logger.hpp"

#include <iostream>

namespace visos {

bool Logger::enabled_ = true;

void Logger::logTick(Tick tick) {
    if (!enabled_) return;
    std::cout << "[TICK " << tick << "]\n";
}

void Logger::logStateTransition(PID pid, ProcessState from, ProcessState to) {
    if (!enabled_) return;
    std::cout << "  [STATE] PID " << pid
              << ": " << to_string(from) << " -> " << to_string(to) << "\n";
}

void Logger::logEvent(Tick tick, KernelEvent event, PID pid) {
    if (!enabled_) return;
    std::cout << "  [EVENT] Tick " << tick
              << ": " << to_string(event)
              << " (PID " << pid << ")\n";
}

void Logger::logContextSwitch(PID old_pid, PID new_pid) {
    if (!enabled_) return;
    std::cout << "  [CONTEXT SWITCH] PID " << old_pid
              << " -> PID " << new_pid << "\n";
}

void Logger::logProcessCreated(PID pid, std::string_view program_name) {
    if (!enabled_) return;
    std::cout << "  [PROCESS CREATED] PID " << pid
              << " (" << program_name << ")\n";
}

void Logger::logSchedulerInvoked(KernelEvent reason) {
    if (!enabled_) return;
    std::cout << "  [SCHEDULER] Invoked due to " << to_string(reason) << "\n";
}

void Logger::logMessage(std::string_view message) {
    if (!enabled_) return;
    std::cout << "  [INFO] " << message << "\n";
}

} // namespace visos
