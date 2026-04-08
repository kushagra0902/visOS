#pragma once

#include <QObject>
#include <QString>
#include <QVector>

struct StateSegment {
    qint64 startTick;
    qint64 endTick;   // -1 means still ongoing
    QString state;
};

struct GanttProcess {
    qint32 pid;
    QString programName;   // set via setProcessName()
    QVector<StateSegment> segments;
};

class GanttModel : public QObject {
    Q_OBJECT

public:
    explicit GanttModel(QObject* parent = nullptr);

    [[nodiscard]] const QVector<GanttProcess>& processes() const noexcept;
    [[nodiscard]] qint64 maxTick() const noexcept;

    // Generates a human-readable narrative of the simulation so far.
    [[nodiscard]] QString generateReport() const;

    // Call after submitting a program so the report can use real names.
    void setProcessName(qint32 pid, const QString& name);

public slots:
    void onProcessCreated(qint32 pid, QString state, int priority, qint64 estCpu);
    void onStateTransitioned(qint32 pid, QString from, QString to);
    void onTickCompleted(qint64 tick);
    void clear();

private:
    GanttProcess* findProcess(qint32 pid);
    [[nodiscard]] const GanttProcess* findProcess(qint32 pid) const;

    QVector<GanttProcess> processes_;
    qint64 currentTick_ = 0;
};
