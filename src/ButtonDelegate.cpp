#include "ButtonDelegate.h"
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QToolTip>
#include <QMouseEvent>
#include <QAbstractItemView>

ButtonDelegate::ButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, nullptr);

    QString progressText = index.data(Qt::DisplayRole).toString();

    int buttonWidth = 45;
    int buttonHeight = 20;
    int spacing = 5;
    int y = opt.rect.y() + (opt.rect.height() - buttonHeight) / 2;

    m_doneButtonRect = QRect(opt.rect.right() - buttonWidth - spacing, y, buttonWidth, buttonHeight);
    m_resetButtonRect = QRect(m_doneButtonRect.left() - buttonWidth - spacing, y, buttonWidth, buttonHeight);

    QRect textRect = opt.rect;
    textRect.setRight(m_resetButtonRect.left() - spacing);
    textRect.setLeft(textRect.left() + 5);

    painter->save();
    if (opt.state & QStyle::State_Selected) {
        painter->setPen(opt.palette.highlightedText().color());
    } else {
        painter->setPen(opt.palette.text().color());
    }
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, progressText);
    painter->restore();

    QStyleOptionButton resetButtonOption;
    resetButtonOption.rect = m_resetButtonRect;
    resetButtonOption.text = tr("Đặt lại");
    resetButtonOption.state = QStyle::State_Enabled;
    if (opt.state & QStyle::State_Selected) resetButtonOption.palette = opt.palette;
    if (m_pressedIndex == index && m_pressedButton == 0) {
        resetButtonOption.state |= QStyle::State_Sunken;
    }
    QApplication::style()->drawControl(QStyle::CE_PushButton, &resetButtonOption, painter);

    QStyleOptionButton doneButtonOption;
    doneButtonOption.rect = m_doneButtonRect;
    doneButtonOption.text = tr("Xong");
    doneButtonOption.state = QStyle::State_Enabled;
    if (opt.state & QStyle::State_Selected) doneButtonOption.palette = opt.palette;
    if (m_pressedIndex == index && m_pressedButton == 1) {
        doneButtonOption.state |= QStyle::State_Sunken;
    }
    QApplication::style()->drawControl(QStyle::CE_PushButton, &doneButtonOption, painter);
}

bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        bool onReset = m_resetButtonRect.contains(mouseEvent->pos());
        bool onDone = m_doneButtonRect.contains(mouseEvent->pos());
        
        if (onReset || onDone) {
            if (event->type() == QEvent::MouseButtonPress) {
                m_pressedIndex = index;
                m_pressedButton = onReset ? 0 : 1;
            } else { 
                if (m_pressedIndex == index) {
                    if (m_pressedButton == 0 && onReset) emit resetClicked(index);
                    if (m_pressedButton == 1 && onDone) emit doneClicked(index);
                }
                m_pressedIndex = QModelIndex();
                m_pressedButton = -1;
            }
            
            if (auto* view = qobject_cast<QAbstractItemView*>(parent())) {
                view->update(index);
            }
            return true;
        }
    }
    
    // THÊM TOOLTIP
    else if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
        if (m_resetButtonRect.contains(helpEvent->pos())) {
            QToolTip::showText(helpEvent->globalPos(), tr("Đặt lại tiến độ chương này về 0%"));
            return true;
        }
        if (m_doneButtonRect.contains(helpEvent->pos())) {
            QToolTip::showText(helpEvent->globalPos(), tr("Đánh dấu đã nghe xong chương này (100%)"));
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonRelease && m_pressedIndex.isValid()) {
        m_pressedIndex = QModelIndex();
        m_pressedButton = -1;
        if (auto* view = qobject_cast<QAbstractItemView*>(parent())) {
            view->update(index);
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
