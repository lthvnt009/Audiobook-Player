#pragma once

#include <QDialog>

class QSpinBox;
class QPushButton;

class SleepTimerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SleepTimerDialog(QWidget *parent = nullptr);

signals:
    void timerSet(int minutes);

private slots:
    void onPresetButtonClicked(int minutes);
    void onOkClicked();

private:
    QSpinBox *m_minutesSpinBox;
};
