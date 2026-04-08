#pragma once

#include "models/queue_model.hpp"

#include <QWidget>
#include <QGroupBox>
#include <QListView>

class QueueViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit QueueViewWidget(QueueModel* readyModel, QueueModel* blockedModel,
                             QWidget* parent = nullptr);

    void updateCounts(int readyCount, int blockedCount);

private:
    QGroupBox* readyGroup_;
    QGroupBox* blockedGroup_;
    QListView* readyView_;
    QListView* blockedView_;
};
