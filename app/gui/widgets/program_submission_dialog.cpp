#include "widgets/program_submission_dialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>

ProgramSubmissionDialog::ProgramSubmissionDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Add Program");
    setMinimumWidth(450);

    auto* mainLayout = new QVBoxLayout(this);

    // Preset selector
    auto* presetLayout = new QHBoxLayout;
    presetLayout->addWidget(new QLabel("Preset:"));
    presetCombo_ = new QComboBox;
    presetCombo_->addItem("Custom");
    presetCombo_->addItem("Calculator (CPU-bound, 5 ticks)");
    presetCombo_->addItem("FileReader (I/O at tick 3)");
    presetCombo_->addItem("WebServer (2 I/O calls)");
    presetLayout->addWidget(presetCombo_, 1);
    mainLayout->addLayout(presetLayout);

    // Program details form
    auto* formGroup = new QGroupBox("Program Details");
    auto* formLayout = new QFormLayout(formGroup);

    nameEdit_ = new QLineEdit;
    nameEdit_->setPlaceholderText("Enter program name");
    formLayout->addRow("Name:", nameEdit_);

    cpuTimeSpin_ = new QSpinBox;
    cpuTimeSpin_->setRange(1, 100);
    cpuTimeSpin_->setValue(5);
    formLayout->addRow("Est. CPU Time:", cpuTimeSpin_);

    memorySpin_ = new QSpinBox;
    memorySpin_->setRange(1, 1024);
    memorySpin_->setValue(64);
    memorySpin_->setSuffix(" MB");
    formLayout->addRow("Memory:", memorySpin_);

    ioDevicesSpin_ = new QSpinBox;
    ioDevicesSpin_->setRange(0, 10);
    ioDevicesSpin_->setValue(0);
    formLayout->addRow("I/O Devices:", ioDevicesSpin_);

    mainLayout->addWidget(formGroup);

    // Syscall profile
    auto* syscallGroup = new QGroupBox("Syscall Profile");
    auto* syscallLayout = new QVBoxLayout(syscallGroup);

    syscallTable_ = new QTableWidget(0, 3);
    syscallTable_->setHorizontalHeaderLabels({"CPU Tick", "Type", "Duration"});
    syscallTable_->horizontalHeader()->setStretchLastSection(true);
    syscallTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    syscallLayout->addWidget(syscallTable_);

    auto* syscallButtons = new QHBoxLayout;
    addSyscallButton_ = new QPushButton("Add Syscall");
    removeSyscallButton_ = new QPushButton("Remove Syscall");
    syscallButtons->addWidget(addSyscallButton_);
    syscallButtons->addWidget(removeSyscallButton_);
    syscallButtons->addStretch();
    syscallLayout->addLayout(syscallButtons);

    mainLayout->addWidget(syscallGroup);

    // OK / Cancel
    auto* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    // Connections
    connect(presetCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProgramSubmissionDialog::onPresetSelected);
    connect(addSyscallButton_, &QPushButton::clicked,
            this, &ProgramSubmissionDialog::addSyscallRow);
    connect(removeSyscallButton_, &QPushButton::clicked,
            this, &ProgramSubmissionDialog::removeSyscallRow);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ProgramSubmissionDialog::onPresetSelected(int index) {
    // Clear syscall table
    syscallTable_->setRowCount(0);

    switch (index) {
        case 1: // Calculator
            nameEdit_->setText("Calculator");
            cpuTimeSpin_->setValue(5);
            memorySpin_->setValue(64);
            ioDevicesSpin_->setValue(0);
            break;
        case 2: // FileReader
            nameEdit_->setText("FileReader");
            cpuTimeSpin_->setValue(8);
            memorySpin_->setValue(128);
            ioDevicesSpin_->setValue(1);
            // Add syscall: IoRead at tick 3, duration 4
            addSyscallRow();
            {
                auto* tickSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(0, 0));
                auto* typeCombo = qobject_cast<QComboBox*>(syscallTable_->cellWidget(0, 1));
                auto* durSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(0, 2));
                if (tickSpin) tickSpin->setValue(3);
                if (typeCombo) typeCombo->setCurrentIndex(0); // IoRead
                if (durSpin) durSpin->setValue(4);
            }
            break;
        case 3: // WebServer
            nameEdit_->setText("WebServer");
            cpuTimeSpin_->setValue(12);
            memorySpin_->setValue(256);
            ioDevicesSpin_->setValue(2);
            // Syscall 1: IoRead at tick 4, duration 3
            addSyscallRow();
            {
                auto* tickSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(0, 0));
                auto* typeCombo = qobject_cast<QComboBox*>(syscallTable_->cellWidget(0, 1));
                auto* durSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(0, 2));
                if (tickSpin) tickSpin->setValue(4);
                if (typeCombo) typeCombo->setCurrentIndex(0);
                if (durSpin) durSpin->setValue(3);
            }
            // Syscall 2: IoWrite at tick 9, duration 2
            addSyscallRow();
            {
                auto* tickSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(1, 0));
                auto* typeCombo = qobject_cast<QComboBox*>(syscallTable_->cellWidget(1, 1));
                auto* durSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(1, 2));
                if (tickSpin) tickSpin->setValue(9);
                if (typeCombo) typeCombo->setCurrentIndex(1); // IoWrite
                if (durSpin) durSpin->setValue(2);
            }
            break;
        default: // Custom
            nameEdit_->clear();
            cpuTimeSpin_->setValue(5);
            memorySpin_->setValue(64);
            ioDevicesSpin_->setValue(0);
            break;
    }
}

void ProgramSubmissionDialog::addSyscallRow() {
    int row = syscallTable_->rowCount();
    syscallTable_->insertRow(row);

    auto* tickSpin = new QSpinBox;
    tickSpin->setRange(1, 100);
    tickSpin->setValue(1);
    syscallTable_->setCellWidget(row, 0, tickSpin);

    auto* typeCombo = new QComboBox;
    typeCombo->addItem("IoRead");
    typeCombo->addItem("IoWrite");
    syscallTable_->setCellWidget(row, 1, typeCombo);

    auto* durationSpin = new QSpinBox;
    durationSpin->setRange(1, 50);
    durationSpin->setValue(3);
    syscallTable_->setCellWidget(row, 2, durationSpin);
}

void ProgramSubmissionDialog::removeSyscallRow() {
    int currentRow = syscallTable_->currentRow();
    if (currentRow >= 0) {
        syscallTable_->removeRow(currentRow);
    } else if (syscallTable_->rowCount() > 0) {
        syscallTable_->removeRow(syscallTable_->rowCount() - 1);
    }
}

visos::ProgramMetadata ProgramSubmissionDialog::getMetadata() const {
    visos::ProgramMetadata meta;
    meta.program_name = nameEdit_->text().toStdString();
    meta.estimated_cpu_time = cpuTimeSpin_->value();
    meta.required_resources = {memorySpin_->value(), ioDevicesSpin_->value()};

    // Build syscall profile
    for (int row = 0; row < syscallTable_->rowCount(); ++row) {
        auto* tickSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(row, 0));
        auto* typeCombo = qobject_cast<QComboBox*>(syscallTable_->cellWidget(row, 1));
        auto* durSpin = qobject_cast<QSpinBox*>(syscallTable_->cellWidget(row, 2));

        if (tickSpin && typeCombo && durSpin) {
            visos::SyscallType type = (typeCombo->currentIndex() == 0)
                                          ? visos::SyscallType::IoRead
                                          : visos::SyscallType::IoWrite;
            meta.syscall_profile.addSyscall(
                tickSpin->value(),
                visos::Syscall{type, durSpin->value()});
        }
    }

    return meta;
}
