#pragma once

#include <QMainWindow>
#include "DataModels.h"

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
    // Tín hiệu này dùng để thông báo cho LibraryWidget cập nhật tiến độ khi Player đang chạy
    void progressUpdated(const QString &bookPath, int chapterIndex, qint64 position);

private slots:
    void onPlayRequest(const BookInfo &book, int chapterIndex);
    void onBackToLibraryRequest();
    void onLibraryPathSelected(const QString &path);
    void onPlaybackPositionChanged(const QString &bookPath, int chapterIndex, qint64 position);
    void onPlaybackRateChanged(const QString &bookPath, qreal rate);
    
    // ĐÃ XÓA: Slot này không còn cần thiết vì logic được xử lý trực tiếp trong LibraryWidget
    // void onChapterProgressChanged(const QString &bookPath, int chapterIndex, qint64 position);


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
};
