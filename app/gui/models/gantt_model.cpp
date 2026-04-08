#include "models/gantt_model.hpp"

GanttModel::GanttModel(QObject* parent)
    : QObject(parent) {}

const QVector<GanttProcess>& GanttModel::processes() const noexcept {
    return processes_;
}

qint64 GanttModel::maxTick() const noexcept {
    return currentTick_;
}

void GanttModel::onProcessCreated(qint32 pid, QString /*state*/, int /*priority*/, qint64 /*estCpu*/) {
    GanttProcess proc;
    proc.pid = pid;
    proc.segments.push_back({currentTick_, -1, "NEW"});
    processes_.push_back(std::move(proc));
}

void GanttModel::onStateTransitioned(qint32 pid, QString /*from*/, QString to) {
    GanttProcess* proc = findProcess(pid);
    if (!proc) return;

    if (!proc->segments.isEmpty()) {
        proc->segments.back().endTick = currentTick_;
    }
    proc->segments.push_back({currentTick_, -1, to});
}

void GanttModel::onTickCompleted(qint64 tick) {
    currentTick_ = tick;
}

void GanttModel::setProcessName(qint32 pid, const QString& name) {
    GanttProcess* proc = findProcess(pid);
    if (proc) proc->programName = name;
}

void GanttModel::clear() {
    processes_.clear();
    currentTick_ = 0;
}

GanttProcess* GanttModel::findProcess(qint32 pid) {
    for (auto& proc : processes_) {
        if (proc.pid == pid) return &proc;
    }
    return nullptr;
}

const GanttProcess* GanttModel::findProcess(qint32 pid) const {
    for (const auto& proc : processes_) {
        if (proc.pid == pid) return &proc;
    }
    return nullptr;
}

// ── Report generation ──────────────────────────────────────────────────────────

QString GanttModel::generateReport() const {
    if (processes_.isEmpty()) {
        return "No simulation data yet.\n\nAdd programs and run the simulation to see a narrative report.";
    }

    // Build a timeline of all events across all processes, sorted by tick
    struct Event {
        qint64 tick;
        qint32 pid;
        QString state;
        qint64 endTick;
        QString name;
    };

    QVector<Event> events;
    for (const auto& proc : processes_) {
        const QString label = proc.programName.isEmpty()
                                  ? QString("PID %1").arg(proc.pid)
                                  : QString("%1 (PID %2)").arg(proc.programName).arg(proc.pid);
        for (const auto& seg : proc.segments) {
            const qint64 end = (seg.endTick == -1) ? currentTick_ : seg.endTick;
            events.push_back({seg.startTick, proc.pid, seg.state, end, label});
        }
    }

    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return (a.tick != b.tick) ? (a.tick < b.tick) : (a.pid < b.pid);
    });

    QString report;
    report += "═══════════════════════════════════════════════════════\n";
    report += "           SIMULATION NARRATIVE REPORT\n";
    report += "═══════════════════════════════════════════════════════\n\n";

    // Section 1: Admitted processes
    report += "📋  SETUP\n";
    report += "───────────────────────────────────────────────────────\n";
    for (const auto& proc : processes_) {
        const QString label = proc.programName.isEmpty()
                                  ? QString("PID %1").arg(proc.pid)
                                  : QString("%1 (PID %2)").arg(proc.programName).arg(proc.pid);
        report += QString("  • Process %1 admitted to the system.\n").arg(label);
    }
    report += "\n";

    // Section 2: Chronological narrative
    report += "⏱️   EXECUTION TIMELINE\n";
    report += "───────────────────────────────────────────────────────\n";

    for (const auto& ev : events) {
        if (ev.state == "NEW") continue;  // skip NEW state (already in setup section)

        const qint64 duration = ev.endTick - ev.tick;
        if (duration <= 0) continue;

        if (ev.state == "RUNNING") {
            report += QString("  [Ticks %1–%2]  %3 was RUNNING for %4 tick(s).\n")
                          .arg(ev.tick).arg(ev.endTick).arg(ev.name).arg(duration);
        } else if (ev.state == "READY") {
            report += QString("  [Ticks %1–%2]  %3 was waiting in the READY queue (%4 tick(s)).\n")
                          .arg(ev.tick).arg(ev.endTick).arg(ev.name).arg(duration);
        } else if (ev.state == "BLOCKED") {
            report += QString("  [Ticks %1–%2]  %3 was BLOCKED on I/O for %4 tick(s).\n")
                          .arg(ev.tick).arg(ev.endTick).arg(ev.name).arg(duration);
        } else if (ev.state == "TERMINATED") {
            report += QString("  [Tick %1]       %2 TERMINATED (total CPU ticks used: %3).\n")
                          .arg(ev.tick).arg(ev.name).arg(
                              // sum all RUNNING segments for this process
                              [&]() -> qint64 {
                                  qint64 total = 0;
                                  if (const auto* p = findProcess(ev.pid)) {
                                      for (const auto& seg : p->segments) {
                                          if (seg.state == "RUNNING") {
                                              total += ((seg.endTick == -1 ? currentTick_ : seg.endTick) - seg.startTick);
                                          }
                                      }
                                  }
                                  return total;
                              }());
        }
    }

    // Section 3: Observations about scheduling behaviour
    report += "\n";
    report += "💡  KEY OBSERVATIONS\n";
    report += "───────────────────────────────────────────────────────\n";

    // Count context switches and I/O blocks
    int totalRunningSegments = 0;
    int totalBlockedSegments = 0;
    for (const auto& proc : processes_) {
        for (const auto& seg : proc.segments) {
            if (seg.state == "RUNNING")  ++totalRunningSegments;
            if (seg.state == "BLOCKED")  ++totalBlockedSegments;
        }
    }
    const int contextSwitches = qMax(0, totalRunningSegments - static_cast<int>(processes_.size()));

    report += QString("  • Total processes:         %1\n").arg(processes_.size());
    report += QString("  • Total simulation ticks:  %1\n").arg(currentTick_);
    report += QString("  • Context switches:        %1\n").arg(contextSwitches);
    report += QString("  • I/O blocking events:     %1\n").arg(totalBlockedSegments);

    if (totalBlockedSegments > 0) {
        report += "\n  ℹ️  I/O blocking: When a process issues an I/O syscall it leaves\n"
                  "     the CPU (RUNNING→BLOCKED), allowing other processes to run.\n"
                  "     When the I/O completes it re-enters the READY queue.\n";
    }
    if (contextSwitches > 0) {
        report += "\n  ℹ️  Context switching: Each time a process is preempted (quantum\n"
                  "     expired) or blocked, the CPU saves its state and loads another\n"
                  "     process — this is a context switch.\n";
    }

    report += "\n═══════════════════════════════════════════════════════\n";
    return report;
}
