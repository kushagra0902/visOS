#pragma once

#include "kernel/kernel.hpp"

#include <QObject>
#include <QString>

class KernelObserverAdapter : public QObject, public visos::IKernelObserver {
    Q_OBJECT

public:
    explicit KernelObserverAdapter(QObject* parent = nullptr);

    // IKernelObserver implementation
    void onTickCompleted(visos::Tick tick) override;
    void onProcessCreated(const visos::ProcessControlBlock& pcb) override;
    void onStateTransition(visos::PID pid, visos::ProcessState from, visos::ProcessState to) override;
    void onContextSwitch(visos::PID old_pid, visos::PID new_pid) override;
    void onSchedulerInvoked(visos::KernelEvent reason) override;
    void onSimulationComplete() override;

signals:
    void tickCompleted(qint64 tick);
    void processCreated(qint32 pid, QString state, int priority, qint64 estimatedCpu);
    void stateTransitioned(qint32 pid, QString from, QString to);
    void contextSwitched(qint32 oldPid, qint32 newPid);
    void schedulerInvoked(QString reason);
    void simulationCompleted();
};
