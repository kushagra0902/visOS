#include "widgets/main_window.hpp"
#include "widgets/program_submission_dialog.hpp"
#include "widgets/process_detail_dialog.hpp"
#include "util/logger.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QDockWidget>
#include <QScrollArea>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Disable kernel's console logging — the GUI event log replaces it
    visos::Logger::setEnabled(false);

    setupKernel();
    setupUi();
    connectSignals();
}

void MainWindow::setupKernel() {
    visos::Kernel::Config config{currentPolicy_, currentQuantum_};
    kernel_ = std::make_unique<visos::Kernel>(config);

    observer_ = std::make_shared<KernelObserverAdapter>(this);
    kernel_->addObserver(observer_);

    visos::SimulationEngine::Config engineConfig;
    engineConfig.max_ticks = 1000;
    engine_ = std::make_unique<visos::SimulationEngine>(*kernel_, engineConfig);
}

void MainWindow::setupUi() {
    // Models
    processTableModel_ = new ProcessTableModel(this);
    readyQueueModel_ = new QueueModel(this);
    blockedQueueModel_ = new QueueModel(this);
    ganttModel_ = new GanttModel(this);

    // Controls toolbar
    controlsWidget_ = new SimulationControlsWidget;

    // Process table
    processTableWidget_ = new ProcessTableWidget(processTableModel_);

    // Queue view
    queueViewWidget_ = new QueueViewWidget(readyQueueModel_, blockedQueueModel_);

    // Event log
    eventLogWidget_ = new EventLogWidget;

    // Gantt chart inside a scroll area
    ganttWidget_ = new GanttWidget(ganttModel_);
    auto* ganttScroll = new QScrollArea;
    ganttScroll->setWidget(ganttWidget_);
    ganttScroll->setWidgetResizable(true);

    // Report panel below the Gantt chart
    reportEdit_ = new QTextEdit;
    reportEdit_->setReadOnly(true);
    {
        QFont mono("Menlo", 10);
        mono.setStyleHint(QFont::Monospace);
        reportEdit_->setFont(mono);
    }
    reportEdit_->setPlaceholderText("Run the simulation to see a human-readable narrative here.");

    // Auto-run timer
    autoRunTimer_ = new QTimer(this);
    autoRunTimer_->setInterval(500);

    // Layout: controls on top, tab widget in center, event log at bottom
    auto* centralWidget = new QWidget;
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    mainLayout->addWidget(controlsWidget_);

    // Tab 1: Process table + queue view
    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(processTableWidget_);
    splitter->addWidget(queueViewWidget_);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    // Tab 2: Gantt chart + report (vertical splitter)
    auto* ganttReportSplitter = new QSplitter(Qt::Vertical);
    ganttReportSplitter->addWidget(ganttScroll);
    ganttReportSplitter->addWidget(reportEdit_);
    ganttReportSplitter->setStretchFactor(0, 2); // chart gets 2/3
    ganttReportSplitter->setStretchFactor(1, 1); // report gets 1/3

    auto* tabWidget = new QTabWidget;
    tabWidget->addTab(splitter, "Processes");
    tabWidget->addTab(ganttReportSplitter, "Gantt Chart & Report");

    mainLayout->addWidget(tabWidget, 1);

    // Event log as bottom dock
    auto* logDock = new QDockWidget("Event Log", this);
    logDock->setWidget(eventLogWidget_);
    logDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);

    setCentralWidget(centralWidget);
}

void MainWindow::connectSignals() {
    // Controls -> MainWindow
    connect(controlsWidget_, &SimulationControlsWidget::stepRequested,
            this, &MainWindow::onStepClicked);
    connect(controlsWidget_, &SimulationControlsWidget::playRequested,
            this, &MainWindow::onPlayClicked);
    connect(controlsWidget_, &SimulationControlsWidget::pauseRequested,
            this, &MainWindow::onPauseClicked);
    connect(controlsWidget_, &SimulationControlsWidget::resetRequested,
            this, &MainWindow::onResetClicked);
    connect(controlsWidget_, &SimulationControlsWidget::restartRequested,
            this, &MainWindow::onRestartClicked);
    connect(controlsWidget_, &SimulationControlsWidget::speedChanged,
            this, &MainWindow::onSpeedChanged);
    connect(controlsWidget_, &SimulationControlsWidget::addProgramRequested,
            this, &MainWindow::onAddProgramClicked);

    connect(controlsWidget_, &SimulationControlsWidget::policyChanged,
            this, [this](int index) {
        Q_UNUSED(index);
        int val = controlsWidget_->findChild<QComboBox*>()->currentData().toInt();
        currentPolicy_ = static_cast<visos::SchedulingPolicy>(val);
    });

    connect(controlsWidget_, &SimulationControlsWidget::quantumChanged,
            this, [this](int value) {
        currentQuantum_ = value;
    });

    // Timer -> auto-run tick
    connect(autoRunTimer_, &QTimer::timeout, this, &MainWindow::onAutoRunTick);

    // Observer -> event log
    connect(observer_.get(), &KernelObserverAdapter::processCreated,
            this, [this](qint32 pid, QString state, int priority, qint64 estCpu) {
        Q_UNUSED(state); Q_UNUSED(priority); Q_UNUSED(estCpu);
        logEvent(QString("[Tick %1] Process created: PID %2")
                     .arg(kernel_->currentTick()).arg(pid));
    });

    connect(observer_.get(), &KernelObserverAdapter::stateTransitioned,
            this, [this](qint32 pid, QString from, QString to) {
        logEvent(QString("[Tick %1] PID %2: %3 -> %4")
                     .arg(kernel_->currentTick()).arg(pid).arg(from).arg(to));
    });

    // Wire adapter -> GanttModel
    connect(observer_.get(), &KernelObserverAdapter::processCreated,
            ganttModel_, &GanttModel::onProcessCreated);
    connect(observer_.get(), &KernelObserverAdapter::stateTransitioned,
            ganttModel_, &GanttModel::onStateTransitioned);
    connect(observer_.get(), &KernelObserverAdapter::tickCompleted,
            ganttModel_, &GanttModel::onTickCompleted);

    connect(observer_.get(), &KernelObserverAdapter::contextSwitched,
            this, [this](qint32 oldPid, qint32 newPid) {
        logEvent(QString("[Tick %1] Context switch: PID %2 -> PID %3")
                     .arg(kernel_->currentTick()).arg(oldPid).arg(newPid));
    });

    connect(observer_.get(), &KernelObserverAdapter::schedulerInvoked,
            this, [this](QString reason) {
        logEvent(QString("[Tick %1] Scheduler invoked: %2")
                     .arg(kernel_->currentTick()).arg(reason));
    });

    connect(observer_.get(), &KernelObserverAdapter::simulationCompleted,
            this, &MainWindow::onSimulationComplete);

    // Process table info column
    connect(processTableWidget_, &ProcessTableWidget::processInfoRequested,
            this, &MainWindow::onProcessInfoRequested);
}

void MainWindow::onStepClicked() {
    if (!kernel_->hasActiveProcesses()) {
        logEvent("No active processes. Add programs or reset.");
        return;
    }
    engine_->step();
    refreshViews();
}

void MainWindow::onPlayClicked() {
    if (!kernel_->hasActiveProcesses()) {
        logEvent("No active processes. Add programs or reset.");
        return;
    }
    controlsWidget_->setSimulationRunning(true);
    autoRunTimer_->start();
}

void MainWindow::onPauseClicked() {
    autoRunTimer_->stop();
    controlsWidget_->setSimulationRunning(false);
}

void MainWindow::onRestartClicked() {
    if (submittedPrograms_.empty()) return;

    // Perform a full reset first
    autoRunTimer_->stop();
    controlsWidget_->setSimulationRunning(false);

    processTableModel_->clear();
    readyQueueModel_->clear();
    blockedQueueModel_->clear();
    ganttModel_->clear();
    eventLogWidget_->clear();

    disconnect(observer_.get(), nullptr, this, nullptr);
    disconnect(observer_.get(), nullptr, ganttModel_, nullptr);

    setupKernel();
    connectSignals();

    processTableWidget_->setTick(0);
    queueViewWidget_->updateCounts(0, 0);
    simulationStarted_ = false;

    // Re-submit all previously submitted programs
    for (const auto& prog : submittedPrograms_) {
        auto meta = prog.metadata; // copy
        const QString qname = prog.name;
        auto id = kernel_->submitProgram(std::move(meta));
        kernel_->runProgram(id);
        const auto& procs = kernel_->allProcesses();
        if (!procs.empty()) {
            const qint32 newPid = static_cast<qint32>(procs.back()->pid());
            processTableModel_->setProcessName(newPid, qname);
            ganttModel_->setProcessName(newPid, qname);
        }
    }
    controlsWidget_->setHasPrograms(true);

    refreshViews();
    logEvent(QString("Restarted with %1 program(s).").arg(submittedPrograms_.size()));
}

void MainWindow::onResetClicked() {
    autoRunTimer_->stop();
    controlsWidget_->setSimulationRunning(false);

    // Clear models and stored programs
    processTableModel_->clear();
    readyQueueModel_->clear();
    blockedQueueModel_->clear();
    ganttModel_->clear();
    eventLogWidget_->clear();
    submittedPrograms_.clear();
    controlsWidget_->setHasPrograms(false);

    // Disconnect old observer signals before recreating
    disconnect(observer_.get(), nullptr, this, nullptr);
    disconnect(observer_.get(), nullptr, ganttModel_, nullptr);

    // Recreate kernel and engine
    setupKernel();
    connectSignals();

    processTableWidget_->setTick(0);
    queueViewWidget_->updateCounts(0, 0);
    reportEdit_->clear();
    simulationStarted_ = false;

    logEvent("Simulation reset.");
}

void MainWindow::onAutoRunTick() {
    if (!kernel_->hasActiveProcesses()) {
        autoRunTimer_->stop();
        controlsWidget_->setSimulationRunning(false);
        return;
    }
    engine_->step();
    refreshViews();
}

void MainWindow::onSpeedChanged(int ms) {
    autoRunTimer_->setInterval(ms);
}

void MainWindow::onAddProgramClicked() {
    ProgramSubmissionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        auto metadata = dialog.getMetadata();
        const QString qname = QString::fromStdString(metadata.program_name);

        // Store a copy before moving
        submittedPrograms_.push_back({metadata, qname});
        controlsWidget_->setHasPrograms(true);

        auto id = kernel_->submitProgram(std::move(metadata));
        kernel_->runProgram(id);

        // Assign name to newly added PCB
        const auto& procs = kernel_->allProcesses();
        if (!procs.empty()) {
            const qint32 newPid = static_cast<qint32>(procs.back()->pid());
            processTableModel_->setProcessName(newPid, qname);
            ganttModel_->setProcessName(newPid, qname);
        }

        refreshViews();
        logEvent(QString("Program '%1' submitted and started.").arg(qname));
    }
}

void MainWindow::onProcessInfoRequested(qint32 pid) {
    const QString name = processTableModel_->nameForPid(pid);
    ProcessDetailDialog dlg(static_cast<visos::PID>(pid), *kernel_, name, *ganttModel_, this);
    dlg.exec();
}

void MainWindow::onSimulationComplete() {
    autoRunTimer_->stop();
    controlsWidget_->setSimulationRunning(false);
    logEvent("=== Simulation Complete ===");

    // Show summary
    QString summary = "Simulation Complete!\n\n";
    for (const auto& pcb : kernel_->allProcesses()) {
        summary += QString("PID %1 | State: %2 | CPU Time: %3 | Priority: %4\n")
                       .arg(pcb->pid())
                       .arg(QString::fromUtf8(visos::to_string(pcb->state())))
                       .arg(pcb->cpuTimeUsed())
                       .arg(pcb->priority());
    }
    summary += QString("\nTotal ticks: %1").arg(kernel_->currentTick());

    QMessageBox::information(this, "Simulation Complete", summary);
}

void MainWindow::refreshViews() {
    processTableModel_->updateFromKernel(*kernel_);
    readyQueueModel_->refreshReady(kernel_->memoryManager().readyQueue());
    blockedQueueModel_->refreshBlocked(kernel_->memoryManager().blockedQueue());

    processTableWidget_->setTick(static_cast<qint64>(kernel_->currentTick()));
    queueViewWidget_->updateCounts(
        static_cast<int>(kernel_->memoryManager().readyQueue().size()),
        static_cast<int>(kernel_->memoryManager().blockedQueue().size()));
    ganttWidget_->refresh();
    reportEdit_->setPlainText(ganttModel_->generateReport());
}

void MainWindow::logEvent(const QString& message) {
    eventLogWidget_->appendEvent(message);
}
