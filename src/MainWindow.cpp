#include "MainWindow.h"
#include "LibraryWidget.h"
#include "PlayerWidget.h"
#include "DatabaseManager.h"

#include <QApplication>
#include <QFile>
#include <QStackedWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QDir>
#include <QAction>
#include <QKeySequence>
#include <QFileInfo>
#include <QMessageBox>
#include <QIcon>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Audiobook Player"));
    setWindowIcon(QIcon(":/icons/icon.ico"));
    
    if (!DatabaseManager::instance().openDb()) {
        QMessageBox::critical(this, tr("Lỗi Database"), tr("Không thể khởi tạo cơ sở dữ liệu. Ứng dụng sẽ thoát."));
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }

    m_settings = new QSettings(this);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    libraryWidget = new LibraryWidget(this);
    playerWidget = new PlayerWidget(this);

    stackedWidget->addWidget(libraryWidget);
    stackedWidget->addWidget(playerWidget);

    connect(libraryWidget, &LibraryWidget::playRequest, this, &MainWindow::onPlayRequest);
    connect(playerWidget, &PlayerWidget::backToLibraryRequest, this, &MainWindow::onBackToLibraryRequest);
    connect(libraryWidget, &LibraryWidget::libraryPathSelected, this, &MainWindow::onLibraryPathSelected);
    connect(playerWidget, &PlayerWidget::playbackPositionChanged, this, &MainWindow::onPlaybackPositionChanged);
    connect(playerWidget, &PlayerWidget::playbackRateChanged, this, &MainWindow::onPlaybackRateChanged);
    connect(libraryWidget, &LibraryWidget::settingsChanged, playerWidget, &PlayerWidget::updateToolTips);
    
    connect(this, &MainWindow::progressUpdated, libraryWidget, &LibraryWidget::onProgressUpdated);

    connect(this, &MainWindow::playbackContextChanged, libraryWidget, &LibraryWidget::onPlaybackContextChanged);
    connect(playerWidget, &PlayerWidget::playbackStateChanged, playerWidget, &PlayerWidget::updatePlaybackStatusText);
    

    createActions();
    
    loadSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::applyTheme()
{
    QString theme = m_settings->value("Settings/theme", "light").toString();
    QApplication *app = qApp;

    if (theme == "light") {
        app->setStyleSheet("");
        return;
    }

    QString styleSheetPath = QString(":/styles/%1.qss").arg(theme);
    QFile styleFile(styleSheetPath);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app->setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::createActions()
{
    m_playPauseAction = new QAction(this);
    m_playPauseAction->setShortcut(Qt::Key_Space);
    connect(m_playPauseAction, &QAction::triggered, playerWidget, &PlayerWidget::onPlayPauseClicked);

    m_nextAction = new QAction(this);
    m_nextAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right));
    connect(m_nextAction, &QAction::triggered, playerWidget, &PlayerWidget::onNextChapterClicked);

    m_prevAction = new QAction(this);
    m_prevAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left));
    connect(m_prevAction, &QAction::triggered, playerWidget, &PlayerWidget::onPrevChapterClicked);

    m_rewindAction = new QAction(this);
    m_rewindAction->setShortcut(Qt::Key_Left);
    connect(m_rewindAction, &QAction::triggered, playerWidget, &PlayerWidget::onRewindClicked);

    m_forwardAction = new QAction(this);
    m_forwardAction->setShortcut(Qt::Key_Right);
    connect(m_forwardAction, &QAction::triggered, playerWidget, &PlayerWidget::onForwardClicked);

    addActions({m_playPauseAction, m_nextAction, m_prevAction, m_rewindAction, m_forwardAction});
}

void MainWindow::setActionsEnabled(bool enabled)
{
    m_playPauseAction->setEnabled(enabled);
    m_nextAction->setEnabled(enabled);
    m_prevAction->setEnabled(enabled);
    m_rewindAction->setEnabled(enabled);
    m_forwardAction->setEnabled(enabled);
}

void MainWindow::onPlayRequest(const BookInfo &book, int chapterIndex)
{
    m_currentBook = book;
    m_currentChapterIndex = chapterIndex;
    emit playbackContextChanged(m_currentBook, m_currentChapterIndex);

    m_settings->setValue("PlaybackStatus/bookPath", book.path);
    m_settings->setValue("PlaybackStatus/chapterIndex", chapterIndex);

    stackedWidget->setCurrentWidget(playerWidget);
    setActionsEnabled(true);
    
    const ChapterInfo& chapter = book.chapters[chapterIndex];
    qint64 startPosition = chapter.listenedDuration;

    qreal playbackRate = m_settings->value("BookSpeeds/" + book.path, 1.0).toReal();

    playerWidget->loadBookAndPlay(book, chapterIndex, playbackRate, startPosition);
}

void MainWindow::onBackToLibraryRequest()
{
    if (playerWidget->isBookLoaded()) {
        onPlaybackPositionChanged(
            playerWidget->getCurrentBookPath(),
            playerWidget->getCurrentChapterIndex(),
            playerWidget->getCurrentPosition()
        );
    }

    playerWidget->stopPlayback();
    stackedWidget->setCurrentWidget(libraryWidget);
    
    // ==================== BẮT ĐẦU SỬA LỖI ====================
    // Không xóa trạng thái khi quay lại thư viện.
    // Chỉ tắt phím tắt toàn cục.
    // emit playbackContextChanged vẫn được giữ lại trong onPlayRequest và loadSettings
    // để đảm bảo label được cập nhật đúng lúc.
    // ===================== KẾT THÚC SỬA LỖI =====================

    QString currentBookPath = playerWidget->getCurrentBookPath();
    if (!currentBookPath.isEmpty()) {
        libraryWidget->selectBookByPath(currentBookPath);
        libraryWidget->forceUpdateChapterView();
    }

    setActionsEnabled(false);
}

void MainWindow::onLibraryPathSelected(const QString &path)
{
    m_settings->setValue("Library/path", path);
}

void MainWindow::onPlaybackPositionChanged(const QString &bookPath, int chapterIndex, qint64 position)
{
    emit progressUpdated(bookPath, chapterIndex, position);
}

void MainWindow::onPlaybackRateChanged(const QString &bookPath, qreal rate)
{
    m_settings->setValue("BookSpeeds/" + bookPath, rate);
}

void MainWindow::loadSettings()
{
    stackedWidget->setCurrentWidget(playerWidget);

    const QByteArray geometry = m_settings->value("geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        resize(900, 700);
    }

    QString lastPath = m_settings->value("Library/path").toString();
    if (!lastPath.isEmpty() && QDir(lastPath).exists()) {
        libraryWidget->scanPathOnStartup(lastPath);
        libraryWidget->restoreLayoutState();

        QString lastBookPath = m_settings->value("PlaybackStatus/bookPath").toString();
        int lastChapterIndex = m_settings->value("PlaybackStatus/chapterIndex", 0).toInt();

        BookInfo lastBook = libraryWidget->getBookByPath(lastBookPath);

        if (!lastBook.path.isEmpty() && lastChapterIndex < lastBook.chapters.size()) {
            qreal playbackRate = m_settings->value("BookSpeeds/" + lastBook.path, 1.0).toReal();
            playerWidget->loadBookInfo(lastBook, lastChapterIndex, playbackRate);
            setActionsEnabled(true);
            
            m_currentBook = lastBook;
            m_currentChapterIndex = lastChapterIndex;
            emit playbackContextChanged(m_currentBook, m_currentChapterIndex);

        } else {
            playerWidget->showWelcomeState();
            setActionsEnabled(false);
        }
    } else {
        playerWidget->showWelcomeState();
        setActionsEnabled(false);
        QMessageBox::information(this, tr("Chào mừng"), tr("Chào mừng bạn đến với Audiobook Player!\n\nVui lòng chọn thư mục chứa các cuốn sách của bạn để bắt đầu."));
        libraryWidget->changeLibraryPath();
    }
}

void MainWindow::saveSettings()
{
    m_settings->setValue("geometry", saveGeometry());
    libraryWidget->saveLayoutState();

    if (playerWidget->isBookLoaded()) {
        m_settings->setValue("PlaybackStatus/bookPath", playerWidget->getCurrentBookPath());
        m_settings->setValue("PlaybackStatus/chapterIndex", playerWidget->getCurrentChapterIndex());
    }
}

