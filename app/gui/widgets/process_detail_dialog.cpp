#include "widgets/process_detail_dialog.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QFont>

ProcessDetailDialog::ProcessDetailDialog(visos::PID pid,
                                         const visos::Kernel& kernel,
                                         const QString& programName,
                                         const GanttModel& ganttModel,
                                         QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString("Process Details — PID %1").arg(pid));
    setMinimumWidth(440);

    auto* layout = new QVBoxLayout(this);

    // ── Identity ──────────────────────────────────────────────────────────────
    auto* identGroup = new QGroupBox("Identity");
    auto* form = new QFormLayout(identGroup);

    // Find PCB
    const visos::ProcessControlBlock* pcb = nullptr;
    for (const auto& p : kernel.allProcesses()) {
        if (p->pid() == pid) { pcb = p.get(); break; }
    }

    form->addRow("PID:", new QLabel(QString::number(pid)));
    form->addRow("Program:", new QLabel(programName.isEmpty() ? "(unknown)" : programName));

    if (pcb) {
        form->addRow("Current State:", new QLabel(
            QString::fromUtf8(visos::to_string(pcb->state()))));
        form->addRow("Priority:", new QLabel(QString::number(pcb->priority())));
        form->addRow("CPU Time Used:", new QLabel(
            QString("%1 / %2 ticks").arg(pcb->cpuTimeUsed()).arg(pcb->estimatedCpuTime())));
        form->addRow("Memory Required:", new QLabel(
            QString("%1 MB").arg(pcb->requiredResources().memory_required)));
        form->addRow("I/O Devices:", new QLabel(
            QString::number(pcb->requiredResources().io_devices_required)));
    }
    layout->addWidget(identGroup);

    // ── State History ─────────────────────────────────────────────────────────
    auto* histGroup = new QGroupBox("State History");
    auto* histLayout = new QVBoxLayout(histGroup);

    auto* histEdit = new QTextEdit;
    histEdit->setReadOnly(true);
    histEdit->setMinimumHeight(150);
    QFont monoFont("Menlo", 10);
    monoFont.setStyleHint(QFont::Monospace);
    histEdit->setFont(monoFont);

    // Find this process in the gantt model
    const GanttProcess* ganttProc = nullptr;
    for (const auto& gp : ganttModel.processes()) {
        if (gp.pid == static_cast<qint32>(pid)) { ganttProc = &gp; break; }
    }

    if (ganttProc && !ganttProc->segments.isEmpty()) {
        QString histText;
        for (const auto& seg : ganttProc->segments) {
            const qint64 end = (seg.endTick == -1) ? ganttModel.maxTick() : seg.endTick;
            histText += QString("  Ticks %1–%2  →  %3\n")
                            .arg(seg.startTick, 4)
                            .arg(end, 4)
                            .arg(seg.state);
        }
        histEdit->setPlainText(histText);
    } else {
        histEdit->setPlainText("  (no history recorded yet)");
    }

    histLayout->addWidget(histEdit);
    layout->addWidget(histGroup);

    // ── Close button ──────────────────────────────────────────────────────────
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}
