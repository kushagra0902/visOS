#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>

class EventLogWidget : public QWidget {
    Q_OBJECT

public:
    explicit EventLogWidget(QWidget* parent = nullptr);

public slots:
    void appendEvent(const QString& message);
    void clear();

private:
    QTextEdit* logView_;
    QPushButton* clearButton_;
};
