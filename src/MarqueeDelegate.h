#pragma once

#include <QStyledItemDelegate>
#include <QTimer>
#include <QModelIndex>

class QAbstractItemView;

class MarqueeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit MarqueeDelegate(QAbstractItemView *view, QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
    void onAnimationTimerTimeout();

private:
    QTimer *m_animationTimer;
    QAbstractItemView *m_view;
    
    // Sử dụng mutable để có thể thay đổi các biến này trong hàm paint (vốn là const)
    mutable QModelIndex m_animatedIndex; // Lưu chỉ số của ô đang chạy hiệu ứng
    mutable int m_offset = 0;
};

