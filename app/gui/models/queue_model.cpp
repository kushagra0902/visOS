#include "models/queue_model.hpp"

QueueModel::QueueModel(QObject* parent)
    : QAbstractListModel(parent) {}

int QueueModel::rowCount(const QModelIndex&) const {
    return entries_.size();
}

QVariant QueueModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= entries_.size())
        return {};

    if (role == Qt::DisplayRole)
        return entries_[index.row()];

    return {};
}

void QueueModel::refreshReady(const visos::ReadyQueue& queue) {
    beginResetModel();
    entries_.clear();
    for (const auto& pcb : queue.entries()) {
        entries_.append(QString("PID %1 (priority: %2)")
                            .arg(pcb->pid())
                            .arg(pcb->priority()));
    }
    endResetModel();
}

void QueueModel::refreshBlocked(const visos::BlockedQueue& queue) {
    beginResetModel();
    entries_.clear();
    for (const auto& entry : queue.entries()) {
        entries_.append(QString("PID %1 (wake @ tick %2)")
                            .arg(entry.pcb->pid())
                            .arg(entry.wakeup_tick));
    }
    endResetModel();
}

void QueueModel::clear() {
    beginResetModel();
    entries_.clear();
    endResetModel();
}
