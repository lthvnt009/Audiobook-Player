#include "TimeInputDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTime>
#include <QLabel>

TimeInputDialog::TimeInputDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Nhảy đến Timecode"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel(tr("Nhập thời gian (hh:mm:ss):"));
    m_timeEdit = new QLineEdit();
    m_timeEdit->setInputMask("99:99:99");
    m_timeEdit->setText("00:00:00");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_goButton = new QPushButton(tr("Đi"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_goButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addWidget(label);
    mainLayout->addWidget(m_timeEdit);
    mainLayout->addLayout(buttonLayout);

    connect(m_goButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

qint64 TimeInputDialog::timeInMs() const
{
    QTime time = QTime::fromString(m_timeEdit->text(), "hh:mm:ss");
    return QTime(0, 0, 0).msecsTo(time);
}
