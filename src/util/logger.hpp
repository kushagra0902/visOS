#pragma once

#include "core/types.hpp"
#include "core/process_state.hpp"
#include "core/kernel_event.hpp"

#include <iostream>
#include <string_view>

namespace visos {

class Logger {
public:
    static void logTick(Tick tick);
    static void logStateTransition(PID pid, ProcessState from, ProcessState to);
    static void logEvent(Tick tick, KernelEvent event, PID pid);
    static void logContextSwitch(PID old_pid, PID new_pid);
    static void logProcessCreated(PID pid, std::string_view program_name);
    static void logSchedulerInvoked(KernelEvent reason);
    static void logMessage(std::string_view message);

    static void setEnabled(bool enabled) noexcept { enabled_ = enabled; }
    [[nodiscard]] static bool isEnabled() noexcept { return enabled_; }

private:
    static bool enabled_;
};

} // namespace visos
