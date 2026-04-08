#pragma once

#include "models/process_table_model.hpp"

#include <QWidget>
#include <QTableView>
#include <QLabel>

class ProcessTableWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProcessTableWidget(ProcessTableModel* model, QWidget* parent = nullptr);

    void setTick(qint64 tick);

signals:
    void processInfoRequested(qint32 pid);

private:
    ProcessTableModel* model_;
    QTableView* tableView_;
    QLabel* tickLabel_;
};
