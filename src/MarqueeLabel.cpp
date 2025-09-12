#include "MarqueeLabel.h"

MarqueeLabel::MarqueeLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent), m_offset(0), m_scrollSpeed(30)
{
    // Thiết lập để label có thể co giãn theo chiều ngang
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_fullText = text;
    m_timer.setInterval(1000 / m_scrollSpeed); // Khung hình/giây
    connect(&m_timer, &QTimer::timeout, this, &MarqueeLabel::onTimerTimeout);
}

void MarqueeLabel::setText(const QString &text)
{
    QLabel::setText(text);
    m_fullText = text;
}

// === BẮT ĐẦU SỬA LỖI KÍCH THƯỚC ===
QSize MarqueeLabel::minimumSizeHint() const
{
    // Báo cho layout manager rằng widget này có thể co lại rất nhỏ theo chiều ngang.
    // Điều này cho phép cửa sổ chính thu nhỏ kích thước.
    // Chiều cao được giữ lại dựa trên font chữ.
    return QSize(0, fontMetrics().height());
}
// === KẾT THÚC SỬA LỖI KÍCH THƯỚC ===


void MarqueeLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Nếu không cuộn, vẽ text bình thường và cắt bớt nếu cần (hiển thị '...')
    if (!m_timer.isActive()) {
        painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, fontMetrics().elidedText(m_fullText, Qt::ElideRight, width()));
    } else { // Nếu đang cuộn, vẽ text với offset
        int textWidth = fontMetrics().horizontalAdvance(m_fullText);
        // Vẽ 2 lần để tạo hiệu ứng lặp lại liền mạch
        painter.drawText(m_offset, rect().bottom() - (rect().height() - fontMetrics().ascent()) / 2, m_fullText);
        painter.drawText(m_offset + textWidth + 30, rect().bottom() - (rect().height() - fontMetrics().ascent()) / 2, m_fullText);
    }
}

void MarqueeLabel::enterEvent(QEnterEvent *event)
{
    if (shouldScroll()) {
        m_offset = 0;
        m_timer.start();
    }
    QLabel::enterEvent(event);
}

void MarqueeLabel::leaveEvent(QEvent *event)
{
    m_timer.stop();
    m_offset = 0;
    update(); // Yêu cầu vẽ lại để hiển thị text tĩnh
    QLabel::leaveEvent(event);
}

void MarqueeLabel::onTimerTimeout()
{
    m_offset -= 1;
    int textWidth = fontMetrics().horizontalAdvance(m_fullText);
    if (m_offset <= -(textWidth + 30)) {
        m_offset = 0; // Reset về vị trí ban đầu
    }
    update(); // Yêu cầu vẽ lại
}

bool MarqueeLabel::shouldScroll() const
{
    return fontMetrics().horizontalAdvance(m_fullText) > width();
}

