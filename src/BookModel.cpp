#include "BookModel.h"
#include <algorithm>

// Hàm helper để định dạng thời gian
QString formatTotalTime(qint64 totalSeconds) {
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;
    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

BookModel::BookModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int BookModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_books.count();
}

int BookModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 4;
}

QVariant BookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }

    const BookInfo &book = m_books[index.row()];

    switch (index.column()) {
        case 0: return book.title;
        case 1: return book.author;
        case 2: {
            qint64 totalListenedMs = 0;
            qint64 totalDurationMs = 0;
            for(const auto& chapter : book.chapters) {
                totalListenedMs += chapter.listenedDuration;
                totalDurationMs += chapter.duration * 1000;
            }
            return QString("%1 / %2")
                .arg(formatTotalTime(totalListenedMs / 1000))
                .arg(formatTotalTime(totalDurationMs / 1000));
        }
        case 3: {
            qint64 totalListenedMs = 0;
            qint64 totalDurationMs = 0;
            for(const auto& chapter : book.chapters) {
                totalListenedMs += chapter.listenedDuration;
                totalDurationMs += chapter.duration * 1000;
            }
            if (totalDurationMs == 0) return "0%";
            int percentage = static_cast<int>((totalListenedMs * 100.0) / totalDurationMs);
            return QString::number(percentage) + "%";
        }
        default: return QVariant();
    }
}

QVariant BookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
        case 0: return tr("Tên sách ▲");
        case 1: return tr("Tác giả");
        case 2: return tr("Thời gian nghe");
        case 3: return tr("Tiến độ (%)");
        default: return QVariant();
    }
}

void BookModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    std::sort(m_books.begin(), m_books.end(), [&](const BookInfo& a, const BookInfo& b) {
        QString valA, valB;
        switch (column) {
            case 0:
                valA = a.title;
                valB = b.title;
                break;
            case 1:
                valA = a.author;
                valB = b.author;
                break;
            default:
                return false;
        }

        if (order == Qt::AscendingOrder) {
            return QString::localeAwareCompare(valA, valB) < 0;
        } else {
            return QString::localeAwareCompare(valA, valB) > 0;
        }
    });

    emit layoutChanged();
}

void BookModel::setBooks(const QList<BookInfo> &books)
{
    beginResetModel();
    m_books = books;
    endResetModel();
}

const BookInfo& BookModel::getBookAt(int row) const
{
    return m_books[row];
}

// SỬA LỖI REAL-TIME UPDATE: Hàm mới để cập nhật dữ liệu nội bộ của model
void BookModel::updateBook(const BookInfo& updatedBook)
{
    for (int i = 0; i < m_books.size(); ++i) {
        if (m_books[i].path == updatedBook.path) {
            // Cập nhật dữ liệu trong model
            m_books[i] = updatedBook;
            // Thông báo cho view rằng dữ liệu của cả dòng đã thay đổi
            emit dataChanged(index(i, 0), index(i, columnCount() - 1));
            return;
        }
    }
}
