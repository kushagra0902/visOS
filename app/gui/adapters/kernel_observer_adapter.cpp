#include "adapters/kernel_observer_adapter.hpp"

KernelObserverAdapter::KernelObserverAdapter(QObject* parent)
    : QObject(parent) {}

void KernelObserverAdapter::onTickCompleted(visos::Tick tick) {
    emit tickCompleted(static_cast<qint64>(tick));
}

void KernelObserverAdapter::onProcessCreated(const visos::ProcessControlBlock& pcb) {
    emit processCreated(
        static_cast<qint32>(pcb.pid()),
        QString::fromUtf8(visos::to_string(pcb.state())),
        pcb.priority(),
        static_cast<qint64>(pcb.estimatedCpuTime()));
}

void KernelObserverAdapter::onStateTransition(
    visos::PID pid, visos::ProcessState from, visos::ProcessState to) {
    emit stateTransitioned(
        static_cast<qint32>(pid),
        QString::fromUtf8(visos::to_string(from)),
        QString::fromUtf8(visos::to_string(to)));
}

void KernelObserverAdapter::onContextSwitch(visos::PID old_pid, visos::PID new_pid) {
    emit contextSwitched(
        static_cast<qint32>(old_pid),
        static_cast<qint32>(new_pid));
}

void KernelObserverAdapter::onSchedulerInvoked(visos::KernelEvent reason) {
    emit schedulerInvoked(QString::fromUtf8(visos::to_string(reason)));
}

void KernelObserverAdapter::onSimulationComplete() {
    emit simulationCompleted();
}
