#pragma once

#include <QWidget>
#include <QString>

class CustomToolTip : public QWidget
{
    Q_OBJECT

public:
    explicit CustomToolTip(QWidget *parent = nullptr);
    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // SỬA LỖI: Thêm khai báo cho biến thành viên m_text
    QString m_text;
};

