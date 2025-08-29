#pragma once

#include <QWidget>

class QSlider;
class QDoubleSpinBox;

class SpeedControlPopup : public QWidget
{
    Q_OBJECT

public:
    explicit SpeedControlPopup(QWidget *parent = nullptr);

    void setSpeed(qreal speed);
    qreal speed() const;

signals:
    void speedChanged(qreal speed);

private slots:
    void onSliderValueChanged(int value);
    void onSpinBoxValueChanged(double value);

private:
    QSlider *m_slider;
    QDoubleSpinBox *m_spinBox;
};
