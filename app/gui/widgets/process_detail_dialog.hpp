#pragma once

#include "kernel/kernel.hpp"
#include "models/gantt_model.hpp"

#include <QDialog>
#include <QString>

// Shows full details for a single process.
class ProcessDetailDialog : public QDialog {
    Q_OBJECT

public:
    // pid         — process to display
    // kernel      — live kernel (for current PCB snapshot)
    // programName — stored by MainWindow at submission time
    // ganttModel  — for state history
    ProcessDetailDialog(visos::PID pid,
                        const visos::Kernel& kernel,
                        const QString& programName,
                        const GanttModel& ganttModel,
                        QWidget* parent = nullptr);
};
