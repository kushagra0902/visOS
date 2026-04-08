#pragma once

#include "services/ready_queue.hpp"
#include "services/blocked_queue.hpp"

#include <QAbstractListModel>
#include <QStringList>

class QueueModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit QueueModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void refreshReady(const visos::ReadyQueue& queue);
    void refreshBlocked(const visos::BlockedQueue& queue);
    void clear();

private:
    QStringList entries_;
};
