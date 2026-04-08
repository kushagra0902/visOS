#include "widgets/process_table_widget.hpp"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QFont>

ProcessTableWidget::ProcessTableWidget(ProcessTableModel* model, QWidget* parent)
    : QWidget(parent), model_(model)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* headerLabel = new QLabel("Process Table");
    QFont boldFont = headerLabel->font();
    boldFont.setBold(true);
    boldFont.setPointSize(boldFont.pointSize() + 1);
    headerLabel->setFont(boldFont);
    layout->addWidget(headerLabel);

    tableView_ = new QTableView;
    tableView_->setModel(model);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->setAlternatingRowColors(true);
    tableView_->horizontalHeader()->setStretchLastSection(false);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->verticalHeader()->hide();
    layout->addWidget(tableView_);

    tickLabel_ = new QLabel("Tick: 0");
    QFont tickFont = tickLabel_->font();
    tickFont.setBold(true);
    tickFont.setPointSize(tickFont.pointSize() + 2);
    tickLabel_->setFont(tickFont);
    layout->addWidget(tickLabel_);

    // Emit processInfoRequested when the ColInfo column cell is clicked
    connect(tableView_, &QTableView::clicked, this, [this](const QModelIndex& idx) {
        if (idx.column() == ProcessTableModel::ColInfo) {
            const QVariant pidVar = model_->data(
                model_->index(idx.row(), ProcessTableModel::ColPID),
                Qt::DisplayRole);
            emit processInfoRequested(static_cast<qint32>(pidVar.toInt()));
        }
    });
}

void ProcessTableWidget::setTick(qint64 tick) {
    tickLabel_->setText(QString("Tick: %1").arg(tick));
}
