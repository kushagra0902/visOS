#include "widgets/event_log_widget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QLabel>
#include <QScrollBar>

EventLogWidget::EventLogWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* headerLayout = new QHBoxLayout;
    auto* headerLabel = new QLabel("Event Log");
    QFont boldFont = headerLabel->font();
    boldFont.setBold(true);
    boldFont.setPointSize(boldFont.pointSize() + 1);
    headerLabel->setFont(boldFont);
    headerLayout->addWidget(headerLabel);

    headerLayout->addStretch();

    clearButton_ = new QPushButton("Clear");
    headerLayout->addWidget(clearButton_);
    layout->addLayout(headerLayout);

    logView_ = new QTextEdit;
    logView_->setReadOnly(true);
    QFont monoFont("Menlo", 11);
    monoFont.setStyleHint(QFont::Monospace);
    logView_->setFont(monoFont);
    layout->addWidget(logView_);

    connect(clearButton_, &QPushButton::clicked, this, &EventLogWidget::clear);
}

void EventLogWidget::appendEvent(const QString& message) {
    logView_->append(message);
    // Auto-scroll to bottom
    auto* scrollBar = logView_->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void EventLogWidget::clear() {
    logView_->clear();
}
