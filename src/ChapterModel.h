#pragma once

#include <QAbstractTableModel>
#include "DataModels.h"

class ChapterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ChapterModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setChapters(const QList<ChapterInfo> &chapters);
    void clear();
    void updateChapter(int chapterIndex);

    // Lấy thông tin chương để xử lý logic nút bấm
    const ChapterInfo& getChapterAt(int row) const;

private:
    QList<ChapterInfo> m_chapters;
};
