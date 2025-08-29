#include "SpeedControlPopup.h"
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QSignalBlocker>
#include <QLabel>
#include <QAbstractSpinBox>

SpeedControlPopup::SpeedControlPopup(QWidget *parent)
    : QWidget(parent, Qt::Popup)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    m_slider = new QSlider(Qt::Horizontal);
    m_spinBox = new QDoubleSpinBox();
    
    QLabel *minLabel = new QLabel("0.5x");
    QLabel *maxLabel = new QLabel("3.0x");

    m_slider->setRange(50, 300);
    m_slider->setSingleStep(1);
    m_slider->setTickInterval(25);
    m_slider->setTickPosition(QSlider::TicksBelow);

    m_spinBox->setRange(0.50, 3.00);
    m_spinBox->setSingleStep(0.01);
    m_spinBox->setDecimals(2);
    m_spinBox->setSuffix("x");
    m_spinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    m_spinBox->setFixedWidth(80);

    layout->addWidget(minLabel);
    layout->addWidget(m_slider);
    layout->addWidget(maxLabel);
    layout->addWidget(m_spinBox);

    // **CẬP NHẬT:** Thêm bo tròn
    setStyleSheet(
        "SpeedControlPopup { background-color: #f0f0f0; border: 1px solid #a0a0a0; border-radius: 8px; }"
        "QLabel { border: none; }"
        "QDoubleSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 20px; }"
        "QDoubleSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 20px; }"
    );
    setFixedSize(320, 60);

    connect(m_slider, &QSlider::valueChanged, this, &SpeedControlPopup::onSliderValueChanged);
    connect(m_spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SpeedControlPopup::onSpinBoxValueChanged);
}

void SpeedControlPopup::setSpeed(qreal speed)
{
    QSignalBlocker sliderBlocker(m_slider);
    QSignalBlocker spinBoxBlocker(m_spinBox);
    m_slider->setValue(static_cast<int>(speed * 100));
    m_spinBox->setValue(speed);
}

qreal SpeedControlPopup::speed() const
{
    return m_spinBox->value();
}

void SpeedControlPopup::onSliderValueChanged(int value)
{
    qreal floatValue = value / 100.0;
    QSignalBlocker spinBoxBlocker(m_spinBox);
    m_spinBox->setValue(floatValue);
    emit speedChanged(floatValue);
}

void SpeedControlPopup::onSpinBoxValueChanged(double value)
{
    QSignalBlocker sliderBlocker(m_slider);
    m_slider->setValue(static_cast<int>(value * 100));
    emit speedChanged(value);
}
