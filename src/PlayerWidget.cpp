#include "PlayerWidget.h"
#include "SpeedControlPopup.h"
#include "TimeInputDialog.h"
#include "SleepTimerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QUrl>
#include <QStyle>
#include <QTime>
#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QFontMetrics>
#include <QSizePolicy>
#include <QPixmap>
#include <QSettings>
#include <QMessageBox>
#include <QIcon>

// Hàm helper để định dạng thời gian
QString formatTime(qint64 ms) {
    if (ms < 0) ms = 0;
    qint64 totalSeconds = ms / 1000;
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    }
}

// Hàm helper để định dạng đồng hồ đếm ngược
QString formatCountdown(int totalSeconds) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}


PlayerWidget::PlayerWidget(QWidget *parent) : QWidget(parent)
{
    m_audioEngine = new AudioEngine(this);
    m_speedPopup = new SpeedControlPopup(this);
    m_speedPopup->hide();
    m_timeInputDialog = new TimeInputDialog(this);
    m_sleepTimerDialog = new SleepTimerDialog(this);
    m_settings = new QSettings(this);
    
    m_timeButtonTimer = new QTimer(this);
    m_timeButtonTimer->setInterval(250);
    m_timeButtonTimer->setSingleShot(true);

    m_sleepCountdownTimer = new QTimer(this);
    m_sleepCountdownTimer->setInterval(1000);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout *topLayout = new QHBoxLayout();
    backButton = new QPushButton();
    backButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    topLayout->addWidget(backButton);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    QHBoxLayout *infoLayout = new QHBoxLayout();
    coverLabel = new QLabel();
    coverLabel->setFixedSize(250, 250);
    coverLabel->setStyleSheet("border: 1px solid gray; background-color: #e0e0e0;");
    coverLabel->setAlignment(Qt::AlignCenter);
    
    QVBoxLayout *textInfoLayout = new QVBoxLayout();
    titleLabel = new QLabel(tr("Tên Sách"));
    authorLabel = new QLabel(tr("Tác giả"));
    chapterLabel = new QLabel(tr("Đang phát: Tên Chương"));
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    
    textInfoLayout->addWidget(titleLabel);
    textInfoLayout->addWidget(authorLabel);
    textInfoLayout->addWidget(chapterLabel);
    textInfoLayout->addStretch();
    
    infoLayout->addWidget(coverLabel);
    infoLayout->addLayout(textInfoLayout);
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();

    QHBoxLayout *progressLayout = new QHBoxLayout();
    currentTimeLabel = new QLabel("00:00");
    progressSlider = new QSlider(Qt::Horizontal);
    totalTimeLabel = new QLabel("00:00");
    progressLayout->addWidget(currentTimeLabel);
    progressLayout->addWidget(progressSlider, 1);
    progressLayout->addWidget(totalTimeLabel);
    mainLayout->addLayout(progressLayout);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    prevChapterButton = new QPushButton();
    rewindButton = new QPushButton();
    playPauseButton = new QPushButton();
    forwardButton = new QPushButton();
    nextChapterButton = new QPushButton();

    const QSize buttonSize(48, 48);
    const QSize playButtonSize(64, 48);
    const QSize iconSize(32, 32);
    playPauseButton->setFixedSize(playButtonSize);
    playPauseButton->setIconSize(iconSize);
    playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    prevChapterButton->setFixedSize(buttonSize);
    prevChapterButton->setIconSize(iconSize);
    prevChapterButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    rewindButton->setFixedSize(buttonSize);
    rewindButton->setIconSize(iconSize);
    rewindButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    forwardButton->setFixedSize(buttonSize);
    forwardButton->setIconSize(iconSize);
    forwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    nextChapterButton->setFixedSize(buttonSize);
    nextChapterButton->setIconSize(iconSize);
    nextChapterButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    muteButton = new QPushButton();
    muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeLabel = new QLabel("100%");
    volumeSlider->setRange(0, 150);
    volumeSlider->setValue(100);
    volumeSlider->setMaximumWidth(100);
    
    controlsLayout->addStretch();
    controlsLayout->addWidget(prevChapterButton);
    controlsLayout->addWidget(rewindButton);
    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(forwardButton);
    controlsLayout->addWidget(nextChapterButton);
    controlsLayout->addStretch();
    controlsLayout->addWidget(muteButton);
    controlsLayout->addWidget(volumeSlider);
    controlsLayout->addWidget(volumeLabel);
    mainLayout->addLayout(controlsLayout);

    QHBoxLayout *subControlsLayout = new QHBoxLayout();
    m_sleepButton = new QPushButton();
    m_sleepTimerLabel = new QLabel();
    speedButton = new QPushButton("1.00x");
    timeDisplayButton = new QPushButton("00:00 / 00:00");

    updateSleepButtonIcon(false);
    m_sleepTimerLabel->setVisible(false);

    subControlsLayout->addStretch();
    subControlsLayout->addWidget(m_sleepButton);
    subControlsLayout->addWidget(m_sleepTimerLabel);
    subControlsLayout->addWidget(speedButton);
    subControlsLayout->addWidget(timeDisplayButton);
    mainLayout->addLayout(subControlsLayout);

    timeDisplayButton->installEventFilter(this);
    updateVolumeSliderStyle(100);
    
    updateToolTips();

    connect(m_audioEngine, &AudioEngine::stateChanged, this, &PlayerWidget::onEngineStateChanged);
    connect(m_audioEngine, &AudioEngine::positionChanged, this, &PlayerWidget::onEnginePositionChanged);
    connect(m_audioEngine, &AudioEngine::durationChanged, this, &PlayerWidget::onEngineDurationChanged);
    connect(m_audioEngine, &AudioEngine::seekFinished, this, &PlayerWidget::onSeekFinished);
    connect(speedButton, &QPushButton::clicked, this, &PlayerWidget::onSpeedButtonClicked);
    connect(m_speedPopup, &SpeedControlPopup::speedChanged, this, &PlayerWidget::onSpeedChanged);
    connect(playPauseButton, &QPushButton::clicked, this, &PlayerWidget::onPlayPauseClicked);
    connect(progressSlider, &QSlider::sliderReleased, this, &PlayerWidget::onProgressSliderReleased);
    connect(progressSlider, &QSlider::valueChanged, this, &PlayerWidget::updateTimeLabels);
    connect(nextChapterButton, &QPushButton::clicked, this, &PlayerWidget::onNextChapterClicked);
    connect(prevChapterButton, &QPushButton::clicked, this, &PlayerWidget::onPrevChapterClicked);
    connect(backButton, &QPushButton::clicked, this, &PlayerWidget::backToLibraryRequest);
    connect(m_timeButtonTimer, &QTimer::timeout, this, &PlayerWidget::onTimeDisplayButtonClicked);
    connect(rewindButton, &QPushButton::clicked, this, &PlayerWidget::onRewindClicked);
    connect(forwardButton, &QPushButton::clicked, this, &PlayerWidget::onForwardClicked);
    connect(muteButton, &QPushButton::clicked, this, &PlayerWidget::onMuteClicked);
    connect(volumeSlider, &QSlider::valueChanged, this, &PlayerWidget::onVolumeSliderChanged);

    connect(m_sleepButton, &QPushButton::clicked, this, &PlayerWidget::onSleepButtonClicked);
    connect(m_sleepTimerDialog, &SleepTimerDialog::timerSet, this, &PlayerWidget::onSetSleepTimer);
    connect(m_sleepCountdownTimer, &QTimer::timeout, this, &PlayerWidget::onSleepTimerTimeout);


    showWelcomeState();
}

void PlayerWidget::updateToolTips()
{
    int seekTime = m_settings->value("Settings/seekSeconds", 15).toInt();
    backButton->setText(tr("Quay lại Thư viện"));
    backButton->setToolTip(tr("Quay lại màn hình thư viện"));
    playPauseButton->setToolTip(tr("Phát/Tạm dừng (Phím cách)"));
    nextChapterButton->setToolTip(tr("Chương kế tiếp (Ctrl+Right)"));
    prevChapterButton->setToolTip(tr("Chương trước (Ctrl+Left)"));
    rewindButton->setToolTip(tr("Tua lùi %1 giây (Left)").arg(seekTime));
    forwardButton->setToolTip(tr("Tua tới %1 giây (Right)").arg(seekTime));
    muteButton->setToolTip(tr("Tắt/Bật tiếng"));
    speedButton->setToolTip(tr("Thay đổi tốc độ phát"));
    timeDisplayButton->setToolTip(tr("Nhấn để đổi hiển thị thời gian, nháy đúp để nhảy đến thời điểm"));
    m_sleepButton->setToolTip(tr("Hẹn giờ tắt"));
}

void PlayerWidget::onSleepButtonClicked()
{
    if (m_sleepCountdownTimer->isActive()) {
        m_sleepCountdownTimer->stop();
        m_sleepTimeRemainingSec = 0;
        updateSleepButtonIcon(false);
        m_sleepTimerLabel->setVisible(false);
        m_audioEngine->setVolume(m_originalVolumeBeforeSleep / 150.0);
        volumeSlider->setValue(m_originalVolumeBeforeSleep);
    } else {
        m_sleepTimerDialog->exec();
    }
}

void PlayerWidget::onSetSleepTimer(int minutes)
{
    if (minutes <= 0) return;

    m_sleepTimeRemainingSec = minutes * 60;
    m_originalVolumeBeforeSleep = volumeSlider->value();
    m_sleepCountdownTimer->start();
    
    updateSleepButtonIcon(true);
    m_sleepTimerLabel->setText(formatCountdown(m_sleepTimeRemainingSec));
    m_sleepTimerLabel->setVisible(true);
}

void PlayerWidget::onSleepTimerTimeout()
{
    m_sleepTimeRemainingSec--;
    m_sleepTimerLabel->setText(formatCountdown(m_sleepTimeRemainingSec));

    if (m_sleepTimeRemainingSec > 0 && m_sleepTimeRemainingSec <= 3) {
        qreal newVolume = m_originalVolumeBeforeSleep * (static_cast<qreal>(m_sleepTimeRemainingSec) / 3.0);
        volumeSlider->setValue(static_cast<int>(newVolume));
    }

    if (m_sleepTimeRemainingSec <= 0) {
        m_sleepCountdownTimer->stop();
        
        if (m_audioEngine->state() == AudioEngine::State::Playing) {
            onPlayPauseClicked();
        }

        updateSleepButtonIcon(false);
        m_sleepTimerLabel->setVisible(false);
        
        QTimer::singleShot(100, this, [this](){
            volumeSlider->setValue(m_originalVolumeBeforeSleep);
        });
    }
}

void PlayerWidget::updateSleepButtonIcon(bool active)
{
    QString iconPath = active ? ":/icons/alarm_active.ico" : ":/icons/alarm_inactive.ico";
    m_sleepButton->setIcon(QIcon(iconPath));
}

void PlayerWidget::seekTo(qint64 positionMs)
{
    if (!isBookLoaded()) return;
    positionMs = qBound(0ll, positionMs, m_duration);
    m_isSeeking = true;
    progressSlider->setValue(positionMs);
    m_audioEngine->seek(positionMs);
}

void PlayerWidget::onRewindClicked()
{
    if (!isBookLoaded()) return;
    qint64 newPosition = progressSlider->value() - (m_settings->value("Settings/seekSeconds", 15).toInt() * 1000);
    seekTo(newPosition);
}

void PlayerWidget::onForwardClicked()
{
    if (!isBookLoaded()) return;
    qint64 newPosition = progressSlider->value() + (m_settings->value("Settings/seekSeconds", 15).toInt() * 1000);
    seekTo(newPosition);
}

bool PlayerWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == timeDisplayButton) {
        if (event->type() == QEvent::MouseButtonPress) {
            m_timeButtonTimer->start();
            return true;
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            m_timeButtonTimer->stop();
            if (m_timeInputDialog->exec() == QDialog::Accepted) {
                seekTo(m_timeInputDialog->timeInMs());
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PlayerWidget::onTimeDisplayButtonClicked()
{
    m_showRemainingTime = !m_showRemainingTime;
    updateTimeLabels();
}

void PlayerWidget::updateTimeLabels()
{
    qint64 position = progressSlider->value();
    qint64 duration = m_duration;
    qint64 remaining = duration - position;

    QString formattedCurrent = formatTime(position);
    QString formattedTotal = formatTime(duration);
    QString formattedRemaining = formatTime(remaining);

    currentTimeLabel->setText(formattedCurrent);

    if (m_showRemainingTime) {
        totalTimeLabel->setText("-" + formattedRemaining);
        timeDisplayButton->setText(QString("-%1 / %2").arg(formattedRemaining, formattedTotal));
    } else {
        totalTimeLabel->setText(formattedTotal);
        timeDisplayButton->setText(QString("%1 / %2").arg(formattedCurrent, formattedTotal));
    }
}

void PlayerWidget::onSpeedButtonClicked()
{
    m_speedPopup->setSpeed(m_audioEngine->rate());
    
    QPoint buttonPos = speedButton->mapToGlobal(QPoint(0, 0));
    int popupX = buttonPos.x() + (speedButton->width() / 2) - (m_speedPopup->width() / 2);
    int popupY = buttonPos.y() - m_speedPopup->height() - 5;
    m_speedPopup->move(popupX, popupY);
    
    m_speedPopup->show();
}

void PlayerWidget::onEnginePositionChanged(qint64 positionMs)
{
    if (m_isSeeking || progressSlider->isSliderDown()) return;

    progressSlider->setValue(positionMs);
    emit playbackPositionChanged(m_currentBook.path, m_currentChapterIndex, positionMs);
}

void PlayerWidget::onEngineDurationChanged(qint64 durationMs)
{
    m_duration = durationMs;
    progressSlider->setRange(0, durationMs);
    updateTimeLabels();
}

void PlayerWidget::loadBookInfo(const BookInfo &book, int chapterIndex, qreal playbackRate)
{
    m_currentBook = book;
    m_currentChapterIndex = chapterIndex;
    
    titleLabel->setText(book.title);
    authorLabel->setText(book.author);
    
    if (!book.coverImagePath.isEmpty()) {
        QPixmap cover(book.coverImagePath);
        coverLabel->setPixmap(cover.scaled(coverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        coverLabel->setText(tr("Không có ảnh bìa"));
        coverLabel->setPixmap(QPixmap());
    }

    onSpeedChanged(playbackRate);

    const ChapterInfo &chapter = m_currentBook.chapters[m_currentChapterIndex];
    chapterLabel->setText(tr("Đang phát: %1").arg(chapter.title));
    
    m_duration = chapter.duration * 1000;
    progressSlider->setRange(0, m_duration);
    progressSlider->setValue(chapter.listenedDuration);
    updateTimeLabels();

    playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void PlayerWidget::loadBookAndPlay(const BookInfo &book, int chapterIndex, qreal playbackRate, qint64 startPosition)
{
    loadBookInfo(book, chapterIndex, playbackRate);
    playChapter(chapterIndex, startPosition);
}

void PlayerWidget::showWelcomeState()
{
    m_currentBook = BookInfo();
    m_currentChapterIndex = -1;

    titleLabel->setText("Audiobook Player");
    authorLabel->setText(tr("Chưa có sách nào được chọn"));
    chapterLabel->setText("");
    coverLabel->setText(tr("Không có ảnh bìa"));
    coverLabel->setPixmap(QPixmap());

    progressSlider->setRange(0, 100);
    progressSlider->setValue(0);
    currentTimeLabel->setText("00:00");
    totalTimeLabel->setText("00:00");
    timeDisplayButton->setText("00:00 / 00:00");
    
    playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}


void PlayerWidget::playChapter(int chapterIndex, qint64 startPosition)
{
    if (chapterIndex < 0 || chapterIndex >= m_currentBook.chapters.count()) {
        if (chapterIndex >= m_currentBook.chapters.count()) {
            emit backToLibraryRequest();
        }
        return;
    }
    m_currentChapterIndex = chapterIndex;
    const ChapterInfo &chapter = m_currentBook.chapters[m_currentChapterIndex];

    chapterLabel->setText(tr("Đang phát: %1").arg(chapter.title));
    
    m_audioEngine->play(chapter.filePath);
    if (startPosition > 0) {
        seekTo(startPosition);
    }
}

void PlayerWidget::stopPlayback()
{
    m_audioEngine->stop();
}

void PlayerWidget::onPlayPauseClicked()
{
    if (!isBookLoaded()) return;

    if (!m_audioEngine->isRunning()) {
        playChapter(m_currentChapterIndex, progressSlider->value());
    } else {
        m_audioEngine->togglePause();
    }
}

void PlayerWidget::onProgressSliderReleased()
{
    if (!isBookLoaded()) return;
    seekTo(progressSlider->value());
}

void PlayerWidget::onEngineStateChanged(AudioEngine::State newState)
{
    if (newState == AudioEngine::State::Playing) {
        playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    } else {
        playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }

    if (newState == AudioEngine::State::Finished) {
        if (isBookLoaded() && m_currentChapterIndex >= m_currentBook.chapters.count() - 1) {
            QMessageBox::information(this, tr("Hoàn thành"), tr("Bạn đã hoàn thành cuốn sách '%1'!").arg(m_currentBook.title));
            emit backToLibraryRequest();
        } else {
            onNextChapterClicked();
        }
    }
}

void PlayerWidget::onSpeedChanged(qreal rate)
{
    m_audioEngine->setRate(rate);
    speedButton->setText(QString::number(rate, 'f', 2) + "x");
    if (!m_currentBook.path.isEmpty()) {
        emit playbackRateChanged(m_currentBook.path, rate);
    }
}

void PlayerWidget::onNextChapterClicked()
{
    if (!isBookLoaded()) return;
    playChapter(m_currentChapterIndex + 1, 0);
}

void PlayerWidget::onPrevChapterClicked()
{
    if (!isBookLoaded()) return;
    playChapter(m_currentChapterIndex - 1, 0);
}

void PlayerWidget::onMuteClicked()
{
    m_isMuted = !m_isMuted;
    if (m_isMuted) {
        m_lastVolume = volumeSlider->value();
        volumeSlider->setValue(0);
    } else {
        volumeSlider->setValue(m_lastVolume > 0 ? m_lastVolume : 100);
    }
}

void PlayerWidget::onVolumeSliderChanged(int value)
{
    m_audioEngine->setVolume(value / 150.0);
    updateVolumeSliderStyle(value);
    volumeLabel->setText(QString::number(value) + "%");
    if (value > 0) {
        m_isMuted = false;
        muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    } else {
        m_isMuted = true;
        muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
}

void PlayerWidget::updateVolumeSliderStyle(int value)
{
    QString color;
    if (value <= 100) {
        color = "#4CAF50"; // Green
    } else if (value <= 125) {
        color = "#FFC107"; // Yellow
    } else {
        color = "#F44336"; // Red
    }

    volumeSlider->setStyleSheet(QString(
        "QSlider::groove:horizontal { border: 1px solid #bbb; background: white; height: 8px; border-radius: 4px; }"
        "QSlider::sub-page:horizontal { background: %1; border: 1px solid %1; height: 8px; border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #e0e0e0; border: 1px solid #777; width: 16px; margin: -4px 0; border-radius: 8px; }"
    ).arg(color));
}

bool PlayerWidget::isBookLoaded() const
{
    return m_currentChapterIndex != -1 && !m_currentBook.path.isEmpty();
}

QString PlayerWidget::getCurrentBookPath() const
{
    return m_currentBook.path;
}

int PlayerWidget::getCurrentChapterIndex() const
{
    return m_currentChapterIndex;
}

qint64 PlayerWidget::getCurrentPosition() const
{
    return progressSlider->value();
}

void PlayerWidget::onSeekFinished()
{
    m_isSeeking = false;
}
