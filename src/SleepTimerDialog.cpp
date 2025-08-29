#include "SleepTimerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGridLayout>

SleepTimerDialog::SleepTimerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Hẹn giờ tắt"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Phần nhập liệu
    QHBoxLayout *inputLayout = new QHBoxLayout();
    QLabel *label = new QLabel(tr("Tự động tắt sau:"));
    m_minutesSpinBox = new QSpinBox();
    m_minutesSpinBox->setRange(1, 999);
    m_minutesSpinBox->setSuffix(tr(" phút"));
    m_minutesSpinBox->setValue(30);
    // SỬA GIAO DIỆN: Thêm stylesheet để xếp nút dọc
    m_minutesSpinBox->setStyleSheet(
        "QSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 16px; }"
        "QSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 16px; }"
    );
    inputLayout->addWidget(label);
    inputLayout->addWidget(m_minutesSpinBox);
    inputLayout->addStretch();

    // Phần các nút chọn sẵn
    QGridLayout *presetsLayout = new QGridLayout();
    QList<int> presets = {5, 10, 15, 30, 60, 90};
    for (int i = 0; i < presets.size(); ++i) {
        int minutes = presets[i];
        QPushButton *button = new QPushButton(QString::number(minutes));
        connect(button, &QPushButton::clicked, this, [this, minutes](){
            onPresetButtonClicked(minutes);
        });
        presetsLayout->addWidget(button, i / 3, i % 3);
    }

    // Nút OK và Hủy
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Bắt đầu"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Hủy"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SleepTimerDialog::onOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addLayout(inputLayout);
    // SỬA GIAO DIỆN: Bỏ dòng chữ "Hoặc chọn nhanh"
    mainLayout->addLayout(presetsLayout);
    mainLayout->addWidget(buttonBox);
}

void SleepTimerDialog::onPresetButtonClicked(int minutes)
{
    m_minutesSpinBox->setValue(minutes);
}

void SleepTimerDialog::onOkClicked()
{
    emit timerSet(m_minutesSpinBox->value());
    accept();
}
