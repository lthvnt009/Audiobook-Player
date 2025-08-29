#pragma once

#include <QStyledItemDelegate>
#include <QModelIndex>

class ButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ButtonDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
    void resetClicked(const QModelIndex &index);
    void doneClicked(const QModelIndex &index);

private:
    mutable QRect m_resetButtonRect;
    mutable QRect m_doneButtonRect;
    
    mutable QModelIndex m_pressedIndex;
    mutable int m_pressedButton = -1;
};
