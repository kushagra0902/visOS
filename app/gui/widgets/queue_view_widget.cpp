#include "widgets/queue_view_widget.hpp"

#include <QVBoxLayout>
#include <QFont>
#include <QLabel>
#include <QSizePolicy>

QueueViewWidget::QueueViewWidget(QueueModel* readyModel, QueueModel* blockedModel,
                                 QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(6);

    auto* headerLabel = new QLabel("Queues");
    QFont boldFont = headerLabel->font();
    boldFont.setBold(true);
    boldFont.setPointSize(boldFont.pointSize() + 1);
    headerLabel->setFont(boldFont);
    layout->addWidget(headerLabel);

    // Ready Queue
    readyGroup_ = new QGroupBox("Ready Queue (0)");
    auto* readyLayout = new QVBoxLayout(readyGroup_);
    readyLayout->setContentsMargins(4, 8, 4, 4);
    readyView_ = new QListView;
    readyView_->setModel(readyModel);
    readyView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    readyView_->setMinimumHeight(100);
    readyView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    readyLayout->addWidget(readyView_);
    readyGroup_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(readyGroup_, 1); // stretch=1 → equal vertical space

    // Blocked Queue
    blockedGroup_ = new QGroupBox("Blocked Queue (0)");
    auto* blockedLayout = new QVBoxLayout(blockedGroup_);
    blockedLayout->setContentsMargins(4, 8, 4, 4);
    blockedView_ = new QListView;
    blockedView_->setModel(blockedModel);
    blockedView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    blockedView_->setMinimumHeight(100);
    blockedView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    blockedLayout->addWidget(blockedView_);
    blockedGroup_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(blockedGroup_, 1); // stretch=1 → equal vertical space
}

void QueueViewWidget::updateCounts(int readyCount, int blockedCount) {
    readyGroup_->setTitle(QString("Ready Queue (%1)").arg(readyCount));
    blockedGroup_->setTitle(QString("Blocked Queue (%1)").arg(blockedCount));
}
