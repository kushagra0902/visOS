#include "widgets/simulation_controls_widget.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

SimulationControlsWidget::SimulationControlsWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // Simulation buttons
    stepButton_ = new QPushButton("Step");
    playButton_ = new QPushButton("Play");
    pauseButton_ = new QPushButton("Pause");
    resetButton_ = new QPushButton("Reset");
    restartButton_ = new QPushButton("↺ Restart");
    restartButton_->setToolTip("Restart simulation with the same programs");
    pauseButton_->setEnabled(false);
    restartButton_->setEnabled(false);

    mainLayout->addWidget(stepButton_);
    mainLayout->addWidget(playButton_);
    mainLayout->addWidget(pauseButton_);
    mainLayout->addWidget(resetButton_);
    mainLayout->addWidget(restartButton_);

    mainLayout->addSpacing(16);

    // Policy selector
    mainLayout->addWidget(new QLabel("Policy:"));
    policyCombo_ = new QComboBox;
    policyCombo_->addItem("Round Robin", 1);
    policyCombo_->addItem("FCFS", 0);
    policyCombo_->addItem("Priority", 2);
    mainLayout->addWidget(policyCombo_);

    // Quantum spinner
    mainLayout->addWidget(new QLabel("Quantum:"));
    quantumSpin_ = new QSpinBox;
    quantumSpin_->setRange(1, 20);
    quantumSpin_->setValue(4);
    mainLayout->addWidget(quantumSpin_);

    mainLayout->addSpacing(16);

    // Speed slider
    mainLayout->addWidget(new QLabel("Speed:"));
    speedSlider_ = new QSlider(Qt::Horizontal);
    speedSlider_->setRange(50, 2000);
    speedSlider_->setValue(500);
    speedSlider_->setFixedWidth(120);
    mainLayout->addWidget(speedSlider_);

    speedLabel_ = new QLabel("500ms");
    speedLabel_->setFixedWidth(50);
    mainLayout->addWidget(speedLabel_);

    mainLayout->addSpacing(16);

    // Add program button
    addProgramButton_ = new QPushButton("Add Program");
    mainLayout->addWidget(addProgramButton_);

    mainLayout->addStretch();

    // Connect signals
    connect(stepButton_, &QPushButton::clicked, this, &SimulationControlsWidget::stepRequested);
    connect(playButton_, &QPushButton::clicked, this, &SimulationControlsWidget::playRequested);
    connect(pauseButton_, &QPushButton::clicked, this, &SimulationControlsWidget::pauseRequested);
    connect(resetButton_, &QPushButton::clicked, this, &SimulationControlsWidget::resetRequested);
    connect(restartButton_, &QPushButton::clicked, this, &SimulationControlsWidget::restartRequested);
    connect(addProgramButton_, &QPushButton::clicked, this, &SimulationControlsWidget::addProgramRequested);

    connect(policyCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SimulationControlsWidget::policyChanged);

    connect(quantumSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SimulationControlsWidget::quantumChanged);

    connect(speedSlider_, &QSlider::valueChanged, this, [this](int value) {
        speedLabel_->setText(QString("%1ms").arg(value));
        emit speedChanged(value);
    });

    // Disable quantum for FCFS
    connect(policyCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        int policyValue = policyCombo_->itemData(index).toInt();
        quantumSpin_->setEnabled(policyValue != 0); // 0 = FCFS
    });
}

void SimulationControlsWidget::setSimulationRunning(bool running) {
    stepButton_->setEnabled(!running);
    playButton_->setEnabled(!running);
    pauseButton_->setEnabled(running);
    policyCombo_->setEnabled(!running);
    quantumSpin_->setEnabled(!running);
    resetButton_->setEnabled(!running);
    restartButton_->setEnabled(!running && restartButton_->property("hasPrograms").toBool());
    addProgramButton_->setEnabled(!running);
}

void SimulationControlsWidget::setHasPrograms(bool has) {
    restartButton_->setProperty("hasPrograms", has);
    restartButton_->setEnabled(has);
}
