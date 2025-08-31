#pragma once

#include <QMainWindow>
#include "DataModels.h"
#include "AudioEngine.h" // Thêm header để sử dụng enum State

class QStackedWidget;
class LibraryWidget;
class PlayerWidget;
class QSettings;
class QCloseEvent;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void applyTheme();

signals:
    void progressUpdated(const QString &bookPath, int chapterIndex, qint64 position);
    // ==================== BẮT ĐẦU THAY ĐỔI ====================
    // Tín hiệu mới để thông báo cho các widget khác về sách/chương đang được mở
    void playbackContextChanged(const BookInfo &book, int chapterIndex);
    // ===================== KẾT THÚC THAY ĐỔI =====================


private slots:
    void onPlayRequest(const BookInfo &book, int chapterIndex);
    void onBackToLibraryRequest();
    void onLibraryPathSelected(const QString &path);
    void onPlaybackPositionChanged(const QString &bookPath, int chapterIndex, qint64 position);
    void onPlaybackRateChanged(const QString &bookPath, qreal rate);
    
private:
    void loadSettings();
    void saveSettings();
    void createActions();
    void setActionsEnabled(bool enabled);

    QStackedWidget *stackedWidget;
    LibraryWidget *libraryWidget;
    PlayerWidget *playerWidget;

    QSettings *m_settings;

    QAction *m_playPauseAction;
    QAction *m_nextAction;
    QAction *m_prevAction;
    QAction *m_rewindAction;
    QAction *m_forwardAction;

    // ==================== BẮT ĐẦU THAY ĐỔI ====================
    // Biến thành viên để lưu trữ thông tin đang phát hiện tại
    BookInfo m_currentBook;
    int m_currentChapterIndex = -1;
    // ===================== KẾT THÚC THAY ĐỔI =====================
};
