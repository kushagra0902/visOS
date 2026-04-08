#include "models/process_table_model.hpp"

ProcessTableModel::ProcessTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int ProcessTableModel::rowCount(const QModelIndex&) const {
    return rows_.size();
}

int ProcessTableModel::columnCount(const QModelIndex&) const {
    return ColumnCount;
}

QVariant ProcessTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rows_.size())
        return {};

    const auto& row = rows_[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColPID:      return row.pid;
            case ColName:     return row.programName.isEmpty() ? QString("PID %1").arg(row.pid) : row.programName;
            case ColState:    return row.state;
            case ColPriority: return row.priority;
            case ColCpuUsed:  return QString("%1 / %2").arg(row.cpuTimeUsed).arg(row.estimatedCpuTime);
            case ColEstCpu:   return row.estimatedCpuTime;
            case ColMemory:   return QString("%1 MB").arg(row.memoryMb);
            case ColIO:       return row.ioDevices;
            case ColInfo:     return QString("ⓘ");
        }
    }

    if (role == Qt::BackgroundRole) {
        return colorForState(row.state);
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColName)
            return static_cast<int>(Qt::AlignVCenter | Qt::AlignLeft);
        return static_cast<int>(Qt::AlignCenter);
    }

    if (role == Qt::ToolTipRole && index.column() == ColInfo) {
        return QString("Click to view full details for PID %1").arg(row.pid);
    }

    if (role == Qt::ForegroundRole && index.column() == ColInfo) {
        return QColor(30, 100, 200); // blue for clickable icon
    }

    return {};
}

QVariant ProcessTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};

    switch (section) {
        case ColPID:      return "PID";
        case ColName:     return "Program";
        case ColState:    return "State";
        case ColPriority: return "Priority";
        case ColCpuUsed:  return "CPU Used";
        case ColEstCpu:   return "Est. CPU";
        case ColMemory:   return "Memory";
        case ColIO:       return "I/O";
        case ColInfo:     return "Details";
    }
    return {};
}

void ProcessTableModel::updateFromKernel(const visos::Kernel& kernel) {
    const auto& processes = kernel.allProcesses();

    if (static_cast<int>(processes.size()) > rows_.size()) {
        beginInsertRows(QModelIndex(), rows_.size(),
                        static_cast<int>(processes.size()) - 1);
        rows_.resize(static_cast<int>(processes.size()));
        endInsertRows();
    }

    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        const auto& pcb = processes[static_cast<std::size_t>(i)];
        auto& row = rows_[i];
        row.pid = static_cast<qint32>(pcb->pid());
        // Preserve stored program name (set via setProcessName)
        row.state = QString::fromUtf8(visos::to_string(pcb->state()));
        row.priority = pcb->priority();
        row.cpuTimeUsed = static_cast<qint64>(pcb->cpuTimeUsed());
        row.estimatedCpuTime = static_cast<qint64>(pcb->estimatedCpuTime());
        row.memoryMb = pcb->requiredResources().memory_required;
        row.ioDevices = pcb->requiredResources().io_devices_required;
    }

    if (!rows_.isEmpty()) {
        emit dataChanged(index(0, 0),
                         index(rows_.size() - 1, ColumnCount - 1));
    }
}

void ProcessTableModel::setProcessName(qint32 pid, const QString& name) {
    for (auto& row : rows_) {
        if (row.pid == pid) {
            row.programName = name;
            return;
        }
    }
    // Process not yet in rows (first call before updateFromKernel) — add a stub
    ProcessRow stub{};
    stub.pid = pid;
    stub.programName = name;
    const int newRow = rows_.size();
    beginInsertRows(QModelIndex(), newRow, newRow);
    rows_.push_back(stub);
    endInsertRows();
}

QString ProcessTableModel::nameForPid(qint32 pid) const {
    for (const auto& row : rows_) {
        if (row.pid == pid) return row.programName;
    }
    return {};
}

void ProcessTableModel::clear() {
    if (rows_.isEmpty()) return;
    beginRemoveRows(QModelIndex(), 0, rows_.size() - 1);
    rows_.clear();
    endRemoveRows();
}

QColor ProcessTableModel::colorForState(const QString& state) {
    if (state == "RUNNING")    return QColor(144, 238, 144, 100);
    if (state == "READY")      return QColor(255, 255, 150, 100);
    if (state == "BLOCKED")    return QColor(255, 160, 160, 100);
    if (state == "TERMINATED") return QColor(200, 200, 200, 100);
    return QColor(255, 255, 255, 100);
}
