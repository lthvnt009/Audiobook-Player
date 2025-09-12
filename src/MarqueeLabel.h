#pragma once

#include <QLabel>
#include <QTimer>
#include <QPainter>
#include <QEnterEvent>
#include <QSizePolicy>

class MarqueeLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MarqueeLabel(const QString &text, QWidget *parent = nullptr);
    void setText(const QString &text);
    const QString& fullText() const { return m_fullText; }

    // === BẮT ĐẦU SỬA LỖI KÍCH THƯỚC ===
    // Ghi đè hàm này để báo cho layout biết kích thước tối thiểu hợp lý
    QSize minimumSizeHint() const override;
    // === KẾT THÚC SỬA LỖI KÍCH THƯỚC ===

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onTimerTimeout();

private:
    bool shouldScroll() const;

    int m_offset;
    int m_scrollSpeed;
    QTimer m_timer;
    QString m_fullText;
};

