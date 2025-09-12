#include "CustomToolTip.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QApplication>
#include <QStyle>

CustomToolTip::CustomToolTip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint)
{
    // Làm cho nền của widget trong suốt để chúng ta có thể vẽ hình dạng tùy chỉnh
    setAttribute(Qt::WA_TranslucentBackground);
    // Kích thước ban đầu
    resize(60, 30);
}

void CustomToolTip::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        // ==================== BẮT ĐẦU CẢI TIẾN ====================
        // Tự động tính toán lại kích thước dựa trên nội dung văn bản mới
        QFontMetrics fm(font());
        QRect textRect = fm.boundingRect(m_text);
        
        // Đặt padding nhỏ hơn để viền sát hơn
        int padding = 8; 
        int arrowHeight = 6;

        // Cập nhật kích thước của widget
        resize(textRect.width() + padding * 2, textRect.height() + padding + arrowHeight);
        // ===================== KẾT THÚC CẢI TIẾN =====================
        
        // Yêu cầu vẽ lại với nội dung và kích thước mới
        update();
    }
}

void CustomToolTip::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ==================== BẮT ĐẦU CẢI TIẾN ====================
    // Sử dụng màu nền và màu văn bản tiêu chuẩn của hệ thống cho tooltip
    // Điều này giúp tooltip trông nhất quán với phần còn lại của ứng dụng
    QColor backgroundColor = palette().color(QPalette::ToolTipBase);
    QColor textColor = palette().color(QPalette::ToolTipText);
    QColor borderColor = palette().color(QPalette::ToolTipText);

    painter.setBrush(backgroundColor);
    painter.setPen(QPen(borderColor, 1)); // Viền mỏng
    // ===================== KẾT THÚC CẢI TIẾN =====================

    QRectF r = rect();
    int arrowHeight = 6;
    int arrowWidth = 12;
    qreal radius = 6.0;

    // Tạo hình dạng của tooltip (hình chữ nhật bo tròn với mũi tên ở dưới)
    QPainterPath path;
    path.moveTo(r.left() + radius, r.top());
    path.lineTo(r.right() - radius, r.top());
    path.arcTo(r.right() - 2 * radius, r.top(), 2 * radius, 2 * radius, 90, -90);
    path.lineTo(r.right(), r.bottom() - arrowHeight - radius);
    path.arcTo(r.right() - 2 * radius, r.bottom() - arrowHeight - 2 * radius, 2 * radius, 2 * radius, 0, -90);
    // Vẽ mũi tên
    path.lineTo(r.center().x() + arrowWidth / 2, r.bottom() - arrowHeight);
    path.lineTo(r.center().x(), r.bottom());
    path.lineTo(r.center().x() - arrowWidth / 2, r.bottom() - arrowHeight);
    // Hoàn thiện hình dạng
    path.lineTo(r.left() + radius, r.bottom() - arrowHeight);
    path.arcTo(r.left(), r.bottom() - arrowHeight - 2 * radius, 2 * radius, 2 * radius, -90, -90);
    path.lineTo(r.left(), r.top() + radius);
    path.arcTo(r.left(), r.top(), 2 * radius, 2 * radius, 180, -90);
    path.closeSubpath();

    painter.drawPath(path);

    // Vẽ văn bản
    painter.setPen(textColor);
    QRect textRect = r.toRect();
    textRect.setHeight(textRect.height() - arrowHeight); // Không vẽ chữ đè lên mũi tên
    painter.drawText(textRect, Qt::AlignCenter, m_text);
}

