#pragma once

#include "core/program_metadata.hpp"
#include "core/syscall.hpp"
#include "core/syscall_profile.hpp"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>

class ProgramSubmissionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProgramSubmissionDialog(QWidget* parent = nullptr);

    visos::ProgramMetadata getMetadata() const;

private slots:
    void onPresetSelected(int index);
    void addSyscallRow();
    void removeSyscallRow();

private:
    QComboBox* presetCombo_;
    QLineEdit* nameEdit_;
    QSpinBox* cpuTimeSpin_;
    QSpinBox* memorySpin_;
    QSpinBox* ioDevicesSpin_;
    QTableWidget* syscallTable_;
    QPushButton* addSyscallButton_;
    QPushButton* removeSyscallButton_;
};
