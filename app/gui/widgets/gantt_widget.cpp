#include "widgets/gantt_widget.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>

GanttWidget::GanttWidget(GanttModel* model, QWidget* parent)
    : QWidget(parent), model_(model)
{
    setMinimumSize(400, 200);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

QSize GanttWidget::sizeHint() const {
    const qint64 maxTick = model_->maxTick();
    const int cols = static_cast<int>(maxTick) + 2;
    const int rows = model_->processes().size();
    const int w = kLabelWidth + cols * kTickWidth + kPadding * 2;
    const int h = kHeaderHeight + rows * kRowHeight + kLegendHeight + kPadding * 2;
    return QSize(w, qMax(h, 200));
}

void GanttWidget::refresh() {
    updateGeometry();  // notify scroll area that size may have changed
    update();          // schedule repaint
}

// ── Paint entry ────────────────────────────────────────────────────────────────

void GanttWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    const auto& procs = model_->processes();
    const qint64 maxTick = model_->maxTick();

    if (procs.isEmpty()) {
        p.drawText(rect(), Qt::AlignCenter, "No processes yet.\nAdd programs and start the simulation.");
        return;
    }

    drawHeader(p, maxTick);
    for (int i = 0; i < procs.size(); ++i) {
        drawRow(p, i, procs[i], maxTick);
    }
    drawLegend(p, procs.size());
}

// ── Header: tick numbers ───────────────────────────────────────────────────────

void GanttWidget::drawHeader(QPainter& p, qint64 maxTick) const {
    const int cols = static_cast<int>(maxTick) + 2;

    // Background
    p.fillRect(0, 0, kLabelWidth + cols * kTickWidth, kHeaderHeight,
               QColor(240, 240, 240));

    // "Tick" label
    p.setPen(Qt::black);
    QFont bold = p.font();
    bold.setBold(true);
    p.setFont(bold);
    p.drawText(QRect(0, 0, kLabelWidth, kHeaderHeight), Qt::AlignCenter, "Tick →");

    // Tick numbers and grid lines — label every 5 ticks (or every tick if small)
    QFont normal = p.font();
    normal.setBold(false);
    normal.setPointSize(normal.pointSize() - 1);
    p.setFont(normal);

    const int step = (maxTick > 40) ? 10 : (maxTick > 20) ? 5 : 1;

    for (qint64 t = 0; t <= maxTick + 1; ++t) {
        const int x = kLabelWidth + static_cast<int>(t) * kTickWidth;

        // Vertical grid line (light, drawn once for the full chart area)
        p.setPen(QColor(200, 200, 200));
        p.drawLine(x, kHeaderHeight,
                   x, kHeaderHeight + static_cast<int>(model_->processes().size()) * kRowHeight);

        // Tick number label
        if (t % step == 0) {
            p.setPen(Qt::darkGray);
            p.drawText(QRect(x, 4, kTickWidth * step, kHeaderHeight - 4),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       QString::number(t));
        }
    }
}

// ── Per-process row ────────────────────────────────────────────────────────────

void GanttWidget::drawRow(QPainter& p, int rowIndex, const GanttProcess& proc, qint64 maxTick) const {
    const int y = kHeaderHeight + rowIndex * kRowHeight;

    // Alternating row background
    p.fillRect(0, y, kLabelWidth, kRowHeight,
               (rowIndex % 2 == 0) ? QColor(248, 248, 248) : QColor(238, 238, 238));

    // PID label
    p.setPen(Qt::black);
    QFont bold = p.font();
    bold.setBold(true);
    p.setFont(bold);
    p.drawText(QRect(kPadding, y, kLabelWidth - kPadding * 2, kRowHeight),
               Qt::AlignVCenter | Qt::AlignLeft,
               QString("PID %1").arg(proc.pid));

    // Row separator line
    p.setPen(QColor(210, 210, 210));
    p.drawLine(0, y + kRowHeight - 1,
               kLabelWidth + static_cast<int>(maxTick + 2) * kTickWidth, y + kRowHeight - 1);

    // State segments
    QFont segFont = p.font();
    segFont.setBold(false);
    segFont.setPointSize(segFont.pointSize() - 1);
    p.setFont(segFont);

    for (const auto& seg : proc.segments) {
        const qint64 start = seg.startTick;
        const qint64 end   = (seg.endTick == -1) ? maxTick + 1 : seg.endTick;
        if (end <= start) continue;

        const int x1 = kLabelWidth + static_cast<int>(start) * kTickWidth;
        const int x2 = kLabelWidth + static_cast<int>(end)   * kTickWidth;
        const int barY = y + 3;
        const int barH = kRowHeight - 6;

        // Fill + border
        const QColor fill = colorForState(seg.state);
        p.fillRect(x1, barY, x2 - x1, barH, fill);
        p.setPen(fill.darker(130));
        p.drawRect(x1, barY, x2 - x1 - 1, barH - 1);

        // State label inside bar (only if wide enough)
        if (x2 - x1 >= 24) {
            p.setPen(Qt::black);
            p.drawText(QRect(x1 + 2, barY, x2 - x1 - 4, barH),
                       Qt::AlignCenter,
                       seg.state.left(3)); // truncate to 3 chars: "RUN", "REA", "BLO", "TER"
        }
    }
}

// ── Legend ─────────────────────────────────────────────────────────────────────

void GanttWidget::drawLegend(QPainter& p, int totalRows) const {
    const int legendY = kHeaderHeight + totalRows * kRowHeight + kPadding;

    p.fillRect(0, legendY, width(), kLegendHeight, QColor(250, 250, 250));

    const struct { const char* label; const char* state; } entries[] = {
        {"RUNNING",    "RUNNING"},
        {"READY",      "READY"},
        {"BLOCKED",    "BLOCKED"},
        {"TERMINATED", "TERMINATED"},
        {"NEW",        "NEW"},
    };

    int x = kLabelWidth;
    const int swatchSize = 14;
    const int gap = 6;

    QFont legFont = p.font();
    legFont.setPointSize(legFont.pointSize() - 1);
    p.setFont(legFont);

    for (const auto& e : entries) {
        const QColor c = colorForState(QString::fromLatin1(e.state));
        p.fillRect(x, legendY + (kLegendHeight - swatchSize) / 2, swatchSize, swatchSize, c);
        p.setPen(c.darker(130));
        p.drawRect(x, legendY + (kLegendHeight - swatchSize) / 2, swatchSize - 1, swatchSize - 1);

        p.setPen(Qt::black);
        const QString label = QString::fromLatin1(e.label);
        const int textW = p.fontMetrics().horizontalAdvance(label) + 8;
        p.drawText(x + swatchSize + 4,
                   legendY + (kLegendHeight + p.fontMetrics().ascent()) / 2 - 1,
                   label);

        x += swatchSize + 4 + textW + gap;
    }
}

// ── Color map ──────────────────────────────────────────────────────────────────

QColor GanttWidget::colorForState(const QString& state) {
    if (state == "RUNNING")    return QColor(100, 210, 100);   // green
    if (state == "READY")      return QColor(255, 220, 60);    // yellow
    if (state == "BLOCKED")    return QColor(255, 110, 110);   // red
    if (state == "TERMINATED") return QColor(180, 180, 180);   // gray
    return QColor(190, 210, 255);                              // blue for NEW
}
