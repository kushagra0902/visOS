#pragma once

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>

class SimulationControlsWidget : public QWidget {
    Q_OBJECT

public:
    explicit SimulationControlsWidget(QWidget* parent = nullptr);

    void setSimulationRunning(bool running);
    void setHasPrograms(bool has);  // enables/disables restart button

signals:
    void stepRequested();
    void playRequested();
    void pauseRequested();
    void resetRequested();
    void restartRequested();   // restart with same programs
    void policyChanged(int index);
    void quantumChanged(int value);
    void speedChanged(int ms);
    void addProgramRequested();

private:
    QPushButton* stepButton_;
    QPushButton* playButton_;
    QPushButton* pauseButton_;
    QPushButton* resetButton_;
    QPushButton* restartButton_;
    QComboBox* policyCombo_;
    QSpinBox* quantumSpin_;
    QSlider* speedSlider_;
    QLabel* speedLabel_;
    QPushButton* addProgramButton_;
};
