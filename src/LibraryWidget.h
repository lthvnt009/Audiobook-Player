#pragma once

#include <QWidget>
#include "DataModels.h"

class QLineEdit;
class QPushButton;
class QTableView;
class QSplitter;
class BookModel;
class ChapterModel;
class QItemSelection;
class QModelIndex;
class QSettings;
class SettingsDialog;
class ButtonDelegate;
class MarqueeLabel;
class MarqueeDelegate;

class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LibraryWidget(QWidget *parent = nullptr);

    void scanPathOnStartup(const QString &path);
    void selectBookByPath(const QString &bookPath);
    ChapterInfo getChapterInfo(const QString& bookPath, int chapterIndex) const;
    void changeLibraryPath();
    BookInfo getBookByPath(const QString &path) const;
    void forceUpdateChapterView();

    void saveLayoutState();
    void restoreLayoutState();

signals:
    void playRequest(const BookInfo &book, int chapterIndex);
    void libraryPathSelected(const QString &path);
    void settingsChanged();
    void progressUpdated(const QString &bookPath, int chapterIndex, qint64 position);
    void scanStateChanged(bool isScanning);


public slots:
    void onProgressUpdated(const QString &bookPath, int chapterIndex, qint64 position);
    void onPlaybackContextChanged(const BookInfo &book, int chapterIndex);


private slots:
    void onRescanLibraryClicked();
    void onBookSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onTableViewDoubleClicked(const QModelIndex &index);
    void onSearchQueryChanged(const QString &text);
    void showBookContextMenu(const QPoint &pos);
    void onRemoveFromLibrary();
    void onDeletePermanently();
    void onSettingsClicked();
    void onChapterResetClicked(const QModelIndex &index);
    void onChapterDoneClicked(const QModelIndex &index);
    void updateUiForScan(bool isScanning);
    void onOpenBookFolder();


private:
    void scanDirectory(const QString &path);
    qint64 getAudioDuration(const QString &filePath);
    QString getMetadata(const QString &filePath, const char* key);
    void updateAndSaveChapterProgress(int chapterId, qint64 newPosition);

    // UI Components
    QLineEdit *searchLineEdit;
    QPushButton *scanLibraryButton;
    QPushButton *settingsButton;
    QTableView *booksTableView;
    QTableView *chaptersTableView;
    QSplitter *mainSplitter;
    SettingsDialog *m_settingsDialog;
    ButtonDelegate *m_buttonDelegate;
    MarqueeLabel *m_statusLabel;
    // ==================== BẮT ĐẦU SỬA LỖI MARQUEE ====================
    // Tạo 2 delegate riêng biệt cho mỗi view
    MarqueeDelegate *m_bookMarqueeDelegate;
    MarqueeDelegate *m_chapterMarqueeDelegate;
    // ===================== KẾT THÚC SỬA LỖI MARQUEE =====================


    // Data Models
    BookModel *m_bookModel;
    ChapterModel *m_chapterModel;

    // Data Storage
    QList<BookInfo> m_allBooks;
    QSettings *m_settings;
    QString m_libraryPath;
};

