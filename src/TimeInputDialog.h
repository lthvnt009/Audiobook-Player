#pragma once

#include <QDialog>

class QLineEdit;
class QPushButton;

class TimeInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeInputDialog(QWidget *parent = nullptr);

    qint64 timeInMs() const;

private:
    QLineEdit *m_timeEdit;
    QPushButton *m_goButton;
};
