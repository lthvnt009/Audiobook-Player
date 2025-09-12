#include "MarqueeDelegate.h"
#include <QAbstractItemView>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QFontMetrics>

MarqueeDelegate::MarqueeDelegate(QAbstractItemView *view, QObject *parent)
    : QStyledItemDelegate(parent), m_view(view)
{
    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(30); // Tốc độ cuộn, tương đương khoảng 33 FPS
    connect(m_animationTimer, &QTimer::timeout, this, &MarqueeDelegate::onAnimationTimerTimeout);
}

void MarqueeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

    // Bước 1: Vẽ nền của ô (bao gồm cả khi được chọn)
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    QRect textRect = opt.rect.adjusted(5, 0, -5, 0);
    const QString text = index.data(Qt::DisplayRole).toString();
    QFontMetrics fontMetrics(opt.font);
    int textWidth = fontMetrics.horizontalAdvance(text);

    bool isHovered = (opt.state & QStyle::State_MouseOver);
    bool needsScrolling = (textWidth > textRect.width());

    // Bước 2: Quản lý trạng thái animation một cách chủ động
    if (isHovered && needsScrolling) {
        // Nếu chuột đang ở trên ô này và nó cần cuộn...
        if (!m_animationTimer->isActive() || m_animatedIndex != index) {
            // ... và animation chưa chạy cho ô này, thì BẮT ĐẦU.
            m_offset = 0;
            m_animatedIndex = index;
            m_animationTimer->start();
        }
    } else if (m_animatedIndex == index) {
        // Nếu ô này đang được animate nhưng chuột không còn ở trên...
        // ... thì DỪNG animation.
        m_animationTimer->stop();
        m_animatedIndex = QModelIndex(); // Reset trạng thái
    }

    // Bước 3: Vẽ văn bản dựa trên trạng thái đã được quyết định
    if (m_animatedIndex == index && m_animationTimer->isActive())
    {
        // Vẽ văn bản đang cuộn
        painter->save();
        painter->setClipRect(opt.rect);

        if (opt.state & QStyle::State_Selected) {
            painter->setPen(opt.palette.highlightedText().color());
        } else {
            painter->setPen(opt.palette.text().color());
        }
        
        int y_pos = textRect.y() + (textRect.height() - fontMetrics.height()) / 2 + fontMetrics.ascent();

        painter->drawText(textRect.x() + m_offset, y_pos, text);
        painter->drawText(textRect.x() + m_offset + textWidth + 30, y_pos, text);
        
        painter->restore();
    }
    else
    {
        // Vẽ văn bản tĩnh có dấu "..."
        QString elidedText = fontMetrics.elidedText(text, Qt::ElideRight, textRect.width());
        style->drawItemText(painter, textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.palette, true, elidedText);
    }
}

void MarqueeDelegate::onAnimationTimerTimeout()
{
    // Nếu không có ô nào đang animate, dừng lại (để phòng ngừa)
    if (!m_animatedIndex.isValid()) {
        m_animationTimer->stop();
        return;
    }

    // Cập nhật vị trí cuộn
    m_offset -= 1;

    QFontMetrics fontMetrics(m_view->font());
    int textWidth = fontMetrics.horizontalAdvance(m_animatedIndex.data(Qt::DisplayRole).toString());

    // Reset vị trí cuộn để tạo vòng lặp
    if (m_offset < -(textWidth + 30)) {
        m_offset = 0;
    }

    // Yêu cầu vẽ lại ô đang chạy hiệu ứng
    m_view->viewport()->update(m_view->visualRect(m_animatedIndex));
}

