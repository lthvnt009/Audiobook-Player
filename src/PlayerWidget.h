#pragma once

#include <QWidget>
#include "DataModels.h"
#include "AudioEngine.h"

// Forward declarations
class QPushButton;
class QLabel;
class QSlider;
class SpeedControlPopup;
class TimeInputDialog;
class SleepTimerDialog; // Thêm khai báo cho dialog mới
class QTimer;
class QSettings;

class PlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerWidget(QWidget *parent = nullptr);

    void loadBookAndPlay(const BookInfo &book, int chapterIndex, qreal playbackRate, qint64 startPosition = 0);
    void loadBookInfo(const BookInfo &book, int chapterIndex, qreal playbackRate);
    void showWelcomeState();

    void stopPlayback();

    bool isBookLoaded() const;
    QString getCurrentBookPath() const;
    int getCurrentChapterIndex() const;
    qint64 getCurrentPosition() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void backToLibraryRequest();
    void playbackPositionChanged(const QString &bookPath, int chapterIndex, qint64 position);
    void playbackRateChanged(const QString &bookPath, qreal rate);

public slots:
    void onPlayPauseClicked();
    void onNextChapterClicked();
    void onPrevChapterClicked();
    void onRewindClicked();
    void onForwardClicked();
    void updateToolTips();

private slots:
    void onEngineStateChanged(AudioEngine::State newState);
    void onEnginePositionChanged(qint64 positionMs);
    void onEngineDurationChanged(qint64 durationMs);
    void onMuteClicked();
    void onVolumeSliderChanged(int value);
    void onProgressSliderReleased();
    void onSpeedButtonClicked();
    void onSpeedChanged(qreal rate);
    void onTimeDisplayButtonClicked();
    void onSeekFinished();
    // Các slot mới cho tính năng hẹn giờ
    void onSleepButtonClicked();
    void onSetSleepTimer(int minutes);
    void onSleepTimerTimeout();


private:
    void playChapter(int chapterIndex, qint64 startPosition = 0);
    void updateTimeLabels();
    void updateVolumeSliderStyle(int value);
    void seekTo(qint64 positionMs);
    void updateSleepButtonIcon(bool active); // Hàm helper cập nhật icon

    // Data
    BookInfo m_currentBook;
    int m_currentChapterIndex = -1;
    qint64 m_duration = 0;
    bool m_showRemainingTime = false;
    QTimer *m_timeButtonTimer;
    bool m_isMuted = false;
    int m_lastVolume = 100;
    bool m_isSeeking = false;

    // Data cho hẹn giờ
    QTimer *m_sleepCountdownTimer;
    int m_sleepTimeRemainingSec = 0;
    int m_originalVolumeBeforeSleep = 100;


    // Core
    AudioEngine *m_audioEngine;
    QSettings *m_settings;
    
    // UI
    SpeedControlPopup *m_speedPopup;
    TimeInputDialog *m_timeInputDialog;
    SleepTimerDialog *m_sleepTimerDialog; // Dialog hẹn giờ
    
    QPushButton *backButton;
    QLabel *coverLabel;
    QLabel *titleLabel;
    QLabel *authorLabel;
    QLabel *chapterLabel;
    QSlider *progressSlider;
    QLabel *currentTimeLabel;
    QLabel *totalTimeLabel;
    QPushButton *prevChapterButton;
    QPushButton *rewindButton;
    QPushButton *playPauseButton;
    QPushButton *forwardButton;
    QPushButton *nextChapterButton;
    QPushButton *speedButton;
    QPushButton *muteButton;
    QSlider *volumeSlider;
    QLabel *volumeLabel;
    QPushButton *timeDisplayButton;
    // UI cho hẹn giờ
    QPushButton *m_sleepButton;
    QLabel *m_sleepTimerLabel;
};
