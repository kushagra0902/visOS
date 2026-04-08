#pragma once

#include "models/gantt_model.hpp"

#include <QWidget>
#include <QColor>

// Renders the Gantt chart using QPainter. Wrap this in a QScrollArea.
class GanttWidget : public QWidget {
    Q_OBJECT

public:
    explicit GanttWidget(GanttModel* model, QWidget* parent = nullptr);

    QSize sizeHint() const override;

public slots:
    void refresh();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    static QColor colorForState(const QString& state);

    void drawHeader(QPainter& p, qint64 maxTick) const;
    void drawRow(QPainter& p, int rowIndex, const GanttProcess& proc, qint64 maxTick) const;
    void drawLegend(QPainter& p, int totalRows) const;

    GanttModel* model_;

    static constexpr int kLabelWidth  = 70;   // pixels for "PID N" labels
    static constexpr int kTickWidth   = 20;   // pixels per tick
    static constexpr int kRowHeight   = 30;   // pixels per process row
    static constexpr int kHeaderHeight = 35;  // tick numbers at top
    static constexpr int kLegendHeight = 30;  // colour legend at bottom
    static constexpr int kPadding     = 8;    // inner padding
};
