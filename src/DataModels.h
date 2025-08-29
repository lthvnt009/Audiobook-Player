#pragma once

#include <QString>
#include <QList>
#include <QVariant>

// Cấu trúc lưu thông tin của một chương (một file audio)
struct ChapterInfo {
    int id = -1; // ID trong database
    QString title;
    QString filePath;
    qint64 duration = 0;
    qint64 listenedDuration = 0;
    QString format;
    qint64 size = 0;
};

// Cấu trúc lưu thông tin của một cuốn sách (một thư mục)
struct BookInfo {
    int id = -1; // ID trong database
    QString title;
    QString author;
    QString path;
    QString coverImagePath;
    QList<ChapterInfo> chapters;
};
