#pragma once

#include <QAbstractTableModel>
#include "DataModels.h"

class BookModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit BookModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setBooks(const QList<BookInfo> &books);
    const BookInfo& getBookAt(int row) const;

    // SỬA LỖI REAL-TIME UPDATE: Hàm cập nhật dữ liệu hiệu quả
    void updateBook(const BookInfo& updatedBook);

private:
    QList<BookInfo> m_books;
};
