#pragma once

#include "kernel/kernel.hpp"
#include "kernel/simulation_engine.hpp"
#include "adapters/kernel_observer_adapter.hpp"
#include "models/process_table_model.hpp"
#include "models/queue_model.hpp"
#include "widgets/process_table_widget.hpp"
#include "widgets/queue_view_widget.hpp"
#include "widgets/simulation_controls_widget.hpp"
#include "widgets/event_log_widget.hpp"
#include "widgets/gantt_widget.hpp"
#include "models/gantt_model.hpp"

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <memory>
#include <vector>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onStepClicked();
    void onPlayClicked();
    void onPauseClicked();
    void onResetClicked();
    void onRestartClicked();
    void onAutoRunTick();
    void onSpeedChanged(int ms);
    void onAddProgramClicked();
    void onProcessInfoRequested(qint32 pid);
    void onSimulationComplete();

private:
    void setupKernel();
    void setupUi();
    void connectSignals();
    void refreshViews();
    void logEvent(const QString& message);

    // Simulation state
    visos::SchedulingPolicy currentPolicy_ = visos::SchedulingPolicy::RoundRobin;
    int currentQuantum_ = 4;

    std::unique_ptr<visos::Kernel> kernel_;
    std::unique_ptr<visos::SimulationEngine> engine_;
    std::shared_ptr<KernelObserverAdapter> observer_;

    // Timer for auto-run
    QTimer* autoRunTimer_;

    // Models
    ProcessTableModel* processTableModel_;
    QueueModel* readyQueueModel_;
    QueueModel* blockedQueueModel_;
    GanttModel* ganttModel_;

    // Widgets
    SimulationControlsWidget* controlsWidget_;
    ProcessTableWidget* processTableWidget_;
    QueueViewWidget* queueViewWidget_;
    EventLogWidget* eventLogWidget_;
    GanttWidget* ganttWidget_;
    QTextEdit* reportEdit_;

    bool simulationStarted_ = false;

    // Tracks programs submitted this session so Restart can re-run them
    struct SubmittedProgram {
        visos::ProgramMetadata metadata;
        QString name; // copy for display
    };
    std::vector<SubmittedProgram> submittedPrograms_;
};
