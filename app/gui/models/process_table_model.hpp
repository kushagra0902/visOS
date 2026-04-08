#pragma once

#include "kernel/kernel.hpp"

#include <QAbstractTableModel>
#include <QColor>
#include <QString>
#include <QVector>

struct ProcessRow {
    qint32 pid;
    QString programName;
    QString state;
    int priority;
    qint64 cpuTimeUsed;
    qint64 estimatedCpuTime;
    int memoryMb;
    int ioDevices;
};

class ProcessTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColPID = 0,
        ColName,
        ColState,
        ColPriority,
        ColCpuUsed,
        ColEstCpu,
        ColMemory,
        ColIO,
        ColInfo,
        ColumnCount
    };

    explicit ProcessTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void updateFromKernel(const visos::Kernel& kernel);
    void setProcessName(qint32 pid, const QString& name);
    void clear();

    // Returns stored name for a pid (empty if unknown)
    [[nodiscard]] QString nameForPid(qint32 pid) const;

private:
    static QColor colorForState(const QString& state);
    QVector<ProcessRow> rows_;
};
