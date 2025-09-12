#pragma once

#include <QObject>
#include <QThread>
#include <QAudioSink>
#include <QIODevice>
#include <QMutex>
#include <atomic>
#include <QElapsedTimer>

// Forward declare FFmpeg structs
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwrContext;
struct AVFilterGraph;
struct AVFilterContext;

class AudioEngine : public QThread
{
    Q_OBJECT

public:
    enum class State { Stopped, Playing, Paused, Finished };
    Q_ENUM(State)

    explicit AudioEngine(QObject *parent = nullptr);
    ~AudioEngine();

    void play(const QString &filePath);
    void stop();
    void togglePause();
    void seek(qint64 positionMs);
    void setRate(qreal rate);
    qreal rate() const;
    void setVolume(qreal volume); // Dùng cho fade effect
    void setSoftwareVolume(qreal amplification); // Dùng cho khuếch đại
    
    State state() const;

signals:
    void stateChanged(AudioEngine::State newState);
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void seekFinished();

protected:
    void run() override;

private:
    enum class PauseRequest { None, Suspend, Resume };

    bool initialize(const QString &filePath);
    void cleanup();
    bool initFilterGraph();

    mutable QMutex m_mutex;
    State m_state = State::Stopped;
    QString m_filePath;
    qint64 m_seekRequest = -1;

    // Các biến cho filter
    qreal m_rate = 1.0;
    bool m_rateChanged = false;
    qreal m_softwareVolume = 1.0;

    // Các biến cho fade
    std::atomic<bool> m_fade_out_requested = false;
    std::atomic<PauseRequest> m_pause_request = PauseRequest::None;
    QElapsedTimer m_fadeTimer;
    qreal m_volumeBeforeFade = 1.0;
    // ==================== BẮT ĐẦU CẢI TIẾN PHẢN HỒI ====================
    // Giảm thời gian fade-out để tăng độ phản hồi khi dừng/chuyển bài
    static const int FADE_DURATION_MS = 200; 
    // ===================== KẾT THÚC CẢI TIẾN PHẢN HỒI =====================
    
    QElapsedTimer m_posUpdateTimer;
    static const int POSITION_UPDATE_INTERVAL_MS = 100;

    // FFmpeg components
    AVFormatContext *m_formatCtx = nullptr;
    AVCodecContext *m_codecCtx = nullptr;
    int m_audioStreamIndex = -1;
    AVFrame *m_frame = nullptr;
    AVPacket *m_packet = nullptr;
    
    // Filter components
    AVFilterGraph *m_filterGraph = nullptr;
    AVFilterContext *m_srcFilterCtx = nullptr;
    AVFilterContext *m_sinkFilterCtx = nullptr;
    AVFilterContext *m_volumeFilterCtx = nullptr;

    // Qt Audio components
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_audioDevice = nullptr;
};
