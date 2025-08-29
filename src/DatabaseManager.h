#pragma once

#include <QObject>
#include <QtSql/QSqlDatabase>
#include "DataModels.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    // Lấy instance duy nhất của DatabaseManager (Singleton pattern)
    static DatabaseManager& instance();

    // Mở và khởi tạo database
    bool openDb();

    // Các hàm thao tác với dữ liệu
    bool addOrUpdateBook(BookInfo& book);
    bool addOrUpdateChapter(int bookId, ChapterInfo& chapter);
    QList<BookInfo> getAllBooks();
    void updateChapterProgress(int chapterId, qint64 listenedDuration);
    void removeBook(int bookId);
    void cleanUpOrphanedRecords(const QList<BookInfo>& booksFromDisk);


private:
    // Private constructor để đảm bảo chỉ có 1 instance
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // Ngăn chặn copy
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Khởi tạo cấu trúc bảng
    bool initializeTables();

    QSqlDatabase m_db;
};
