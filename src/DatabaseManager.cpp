#include "DatabaseManager.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::openDb()
{
    if (m_db.isOpen()) {
        return true;
    }

    // Đặt file database trong thư mục dữ liệu của ứng dụng
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_db.setDatabaseName(path + "/library.db");

    if (!m_db.open()) {
        qCritical() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }

    return initializeTables();
}

bool DatabaseManager::initializeTables()
{
    QSqlQuery query;
    bool success = true;

    // Bảng books
    if (!query.exec("CREATE TABLE IF NOT EXISTS books ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "title TEXT NOT NULL, "
                    "author TEXT, "
                    "path TEXT UNIQUE NOT NULL, "
                    "cover_path TEXT"
                    ")")) {
        qCritical() << "Failed to create books table:" << query.lastError().text();
        success = false;
    }

    // Bảng chapters
    if (!query.exec("CREATE TABLE IF NOT EXISTS chapters ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "book_id INTEGER NOT NULL, "
                    "title TEXT NOT NULL, "
                    "file_path TEXT UNIQUE NOT NULL, "
                    "duration INTEGER DEFAULT 0, "
                    "listened_duration INTEGER DEFAULT 0, "
                    "format TEXT, "
                    "size INTEGER DEFAULT 0, "
                    "FOREIGN KEY(book_id) REFERENCES books(id) ON DELETE CASCADE"
                    ")")) {
        qCritical() << "Failed to create chapters table:" << query.lastError().text();
        success = false;
    }
    
    return success;
}

bool DatabaseManager::addOrUpdateBook(BookInfo& book)
{
    QSqlQuery query;
    query.prepare("SELECT id, author, cover_path FROM books WHERE path = :path");
    query.bindValue(":path", book.path);
    if (query.exec() && query.next()) {
        // Sách đã tồn tại, cập nhật và lấy ID
        book.id = query.value(0).toInt();
        QString oldAuthor = query.value(1).toString();
        QString oldCover = query.value(2).toString();

        if (oldAuthor != book.author || oldCover != book.coverImagePath) {
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE books SET author = :author, cover_path = :cover_path WHERE id = :id");
            updateQuery.bindValue(":author", book.author);
            updateQuery.bindValue(":cover_path", book.coverImagePath);
            updateQuery.bindValue(":id", book.id);
            if(!updateQuery.exec()) {
                qWarning() << "Failed to update book:" << updateQuery.lastError().text();
                return false;
            }
        }
    } else {
        // Sách chưa tồn tại, thêm mới
        query.prepare("INSERT INTO books (title, author, path, cover_path) "
                      "VALUES (:title, :author, :path, :cover_path)");
        query.bindValue(":title", book.title);
        query.bindValue(":author", book.author);
        query.bindValue(":path", book.path);
        query.bindValue(":cover_path", book.coverImagePath);
        if (!query.exec()) {
            qWarning() << "Failed to add book:" << query.lastError().text();
            return false;
        }
        book.id = query.lastInsertId().toInt();
    }
    return true;
}

bool DatabaseManager::addOrUpdateChapter(int bookId, ChapterInfo& chapter)
{
    QSqlQuery query;
    query.prepare("SELECT id, listened_duration FROM chapters WHERE file_path = :file_path");
    query.bindValue(":file_path", chapter.filePath);

    if (query.exec() && query.next()) {
        // Chương đã tồn tại, cập nhật và lấy ID
        chapter.id = query.value(0).toInt();
        chapter.listenedDuration = query.value(1).toLongLong(); // Lấy tiến độ đã lưu

        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE chapters SET title = :title, duration = :duration, format = :format, size = :size "
                            "WHERE id = :id");
        updateQuery.bindValue(":title", chapter.title);
        updateQuery.bindValue(":duration", chapter.duration);
        updateQuery.bindValue(":format", chapter.format);
        updateQuery.bindValue(":size", chapter.size);
        updateQuery.bindValue(":id", chapter.id);
        if(!updateQuery.exec()) {
            qWarning() << "Failed to update chapter:" << updateQuery.lastError().text();
            return false;
        }
    } else {
        // Chương chưa tồn tại, thêm mới
        query.prepare("INSERT INTO chapters (book_id, title, file_path, duration, listened_duration, format, size) "
                      "VALUES (:book_id, :title, :file_path, :duration, :listened_duration, :format, :size)");
        query.bindValue(":book_id", bookId);
        query.bindValue(":title", chapter.title);
        query.bindValue(":file_path", chapter.filePath);
        query.bindValue(":duration", chapter.duration);
        query.bindValue(":listened_duration", 0); // Mặc định là 0
        query.bindValue(":format", chapter.format);
        query.bindValue(":size", chapter.size);
        if (!query.exec()) {
            qWarning() << "Failed to add chapter:" << query.lastError().text();
            return false;
        }
        chapter.id = query.lastInsertId().toInt();
    }
    return true;
}

QList<BookInfo> DatabaseManager::getAllBooks()
{
    QList<BookInfo> books;
    QSqlQuery bookQuery("SELECT id, title, author, path, cover_path FROM books ORDER BY title");
    
    while (bookQuery.next()) {
        BookInfo book;
        book.id = bookQuery.value(0).toInt();
        book.title = bookQuery.value(1).toString();
        book.author = bookQuery.value(2).toString();
        book.path = bookQuery.value(3).toString();
        book.coverImagePath = bookQuery.value(4).toString();

        QSqlQuery chapterQuery;
        chapterQuery.prepare("SELECT id, title, file_path, duration, listened_duration, format, size "
                             "FROM chapters WHERE book_id = :book_id ORDER BY file_path");
        chapterQuery.bindValue(":book_id", book.id);
        if (chapterQuery.exec()) {
            while (chapterQuery.next()) {
                ChapterInfo chapter;
                chapter.id = chapterQuery.value(0).toInt();
                chapter.title = chapterQuery.value(1).toString();
                chapter.filePath = chapterQuery.value(2).toString();
                chapter.duration = chapterQuery.value(3).toLongLong();
                chapter.listenedDuration = chapterQuery.value(4).toLongLong();
                chapter.format = chapterQuery.value(5).toString();
                chapter.size = chapterQuery.value(6).toLongLong();
                book.chapters.append(chapter);
            }
        }
        books.append(book);
    }
    return books;
}

void DatabaseManager::updateChapterProgress(int chapterId, qint64 listenedDuration)
{
    QSqlQuery query;
    query.prepare("UPDATE chapters SET listened_duration = :listened_duration WHERE id = :id");
    query.bindValue(":listened_duration", listenedDuration);
    query.bindValue(":id", chapterId);
    if (!query.exec()) {
        qWarning() << "Failed to update progress:" << query.lastError().text();
    }
}

void DatabaseManager::removeBook(int bookId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM books WHERE id = :id");
    query.bindValue(":id", bookId);
    if (!query.exec()) {
        qWarning() << "Failed to remove book:" << query.lastError().text();
    }
}
