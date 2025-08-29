#include "ChapterModel.h"
#include <QTime>
#include <algorithm>
#include <QFileInfo>

ChapterModel::ChapterModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ChapterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_chapters.count();
}

int ChapterModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 5; // Quay về 5 cột, cột cuối cùng sẽ chứa cả tiến độ và nút
}

QVariant ChapterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        const ChapterInfo &chapter = m_chapters[index.row()];
        switch (index.column()) {
            case 0: return chapter.title;
            case 1: return QTime(0, 0).addSecs(chapter.duration).toString("hh:mm:ss");
            case 2: return chapter.format;
            case 3: return QString::number(chapter.size / (1024.0 * 1024.0), 'f', 2) + " MB";
            case 4: {
                if (chapter.duration == 0) return "0%";
                int percentage = static_cast<int>((chapter.listenedDuration * 100.0) / (chapter.duration * 1000.0));
                return QString::number(percentage) + "%";
            }
            default: return QVariant();
        }
    }
    return QVariant();
}

QVariant ChapterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
        case 0: return tr("Tên chương ▲");
        case 1: return tr("Thời lượng");
        case 2: return tr("Định dạng");
        case 3: return tr("Kích thước");
        case 4: return tr("Tiến độ");
        default: return QVariant();
    }
}

void ChapterModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    std::sort(m_chapters.begin(), m_chapters.end(), [&](const ChapterInfo& a, const ChapterInfo& b) {
        bool comparisonResult = false;
        switch (column) {
            case 0:
                comparisonResult = QFileInfo(a.title).baseName().localeAwareCompare(QFileInfo(b.title).baseName()) < 0;
                break;
            case 1:
                comparisonResult = a.duration < b.duration;
                break;
            case 2:
                comparisonResult = a.format < b.format;
                break;
            case 3:
                comparisonResult = a.size < b.size;
                break;
            case 4: {
                double progressA = (a.duration > 0) ? (double)a.listenedDuration / (a.duration * 1000.0) : 0.0;
                double progressB = (b.duration > 0) ? (double)b.listenedDuration / (b.duration * 1000.0) : 0.0;
                comparisonResult = progressA < progressB;
                break;
            }
            default:
                return false;
        }
        return (order == Qt::AscendingOrder) ? comparisonResult : !comparisonResult;
    });

    emit layoutChanged();
}

void ChapterModel::setChapters(const QList<ChapterInfo> &chapters)
{
    beginResetModel();
    m_chapters = chapters;
    endResetModel();
}

void ChapterModel::clear()
{
    beginResetModel();
    m_chapters.clear();
    endResetModel();
}

void ChapterModel::updateChapter(int chapterIndex)
{
    if (chapterIndex >= 0 && chapterIndex < m_chapters.size()) {
        emit dataChanged(index(chapterIndex, 4), index(chapterIndex, 4));
    }
}

const ChapterInfo& ChapterModel::getChapterAt(int row) const
{
    return m_chapters[row];
}
