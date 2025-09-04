#include "AudioEngine.h"
#include <QDebug>
#include <QAudioFormat>
#include <QMutexLocker>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

AudioEngine::AudioEngine(QObject *parent) : QThread(parent) {}

AudioEngine::~AudioEngine()
{
    stop();
    wait();
}

void AudioEngine::play(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);
    if (isRunning()) {
        m_state = State::Stopped;
        m_fade_out_requested = true;
        locker.unlock();
        wait();
        locker.relock();
    }
    m_filePath = filePath;
    m_state = State::Stopped;
    start();
}

void AudioEngine::stop()
{
    m_fade_out_requested = true;
}

void AudioEngine::togglePause()
{
    QMutexLocker locker(&m_mutex);
    if (m_state == State::Playing) {
        m_state = State::Paused;
        m_pause_request = PauseRequest::Suspend;
        emit stateChanged(State::Paused);
    } else if (m_state == State::Paused) {
        m_state = State::Playing;
        if(m_fade_out_requested.load()) {
            m_fade_out_requested = false;
            m_fadeTimer.invalidate();
            if(m_audioSink) m_audioSink->setVolume(m_volumeBeforeFade);
        }
        m_pause_request = PauseRequest::Resume;
        emit stateChanged(State::Playing);
    }
}

void AudioEngine::seek(qint64 positionMs)
{
    QMutexLocker locker(&m_mutex);
    m_seekRequest = positionMs;
}

void AudioEngine::setRate(qreal rate)
{
    QMutexLocker locker(&m_mutex);
    if (m_rate != rate) {
        m_rate = rate;
        m_rateChanged = true;
    }
}

qreal AudioEngine::rate() const
{
    QMutexLocker locker(&m_mutex);
    return m_rate;
}

AudioEngine::State AudioEngine::state() const
{
    QMutexLocker locker(&m_mutex);
    return m_state;
}

// Hàm này chỉ điều khiển âm lượng thiết bị, dùng cho fade
void AudioEngine::setVolume(qreal volume)
{
    if (m_audioSink && !m_fade_out_requested.load()) {
        m_audioSink->setVolume(volume);
    }
}

// ==================== BẮT ĐẦU CẢI TIẾN ÂM LƯỢNG ====================
// Hàm này điều khiển bộ lọc khuếch đại âm thanh
void AudioEngine::setSoftwareVolume(qreal amplification)
{
    QMutexLocker locker(&m_mutex);
    if (m_softwareVolume != amplification) {
        m_softwareVolume = amplification;
        m_volumeChanged = true;
    }
}
// ===================== KẾT THÚC CẢI TIẾN ÂM LƯỢNG =====================

bool AudioEngine::initialize(const QString &filePath)
{
    m_fade_out_requested = false;
    m_fadeTimer.invalidate();
    m_pause_request = PauseRequest::None;

    m_posUpdateTimer.start();

    std::string path_std = filePath.toStdString();
    if (avformat_open_input(&m_formatCtx, path_std.c_str(), nullptr, nullptr) != 0) {
        qCritical() << "Could not open file:" << filePath;
        return false;
    }

    if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
        qCritical() << "Could not find stream info";
        return false;
    }

    const AVCodec *codec = nullptr;
    m_audioStreamIndex = av_find_best_stream(m_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (m_audioStreamIndex < 0 || !codec) {
        qCritical() << "No audio stream found";
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(m_codecCtx, m_formatCtx->streams[m_audioStreamIndex]->codecpar);
    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        qCritical() << "Could not open codec";
        return false;
    }

    m_frame = av_frame_alloc();
    m_packet = av_packet_alloc();

    QAudioFormat format;
    format.setSampleRate(m_codecCtx->sample_rate);
    format.setChannelCount(m_codecCtx->ch_layout.nb_channels);
    format.setSampleFormat(QAudioFormat::Int16);

    m_audioSink = new QAudioSink(format, this);
    m_audioDevice = m_audioSink->start();

    if (!initFilterGraph()) {
        qCritical() << "Failed to initialize filter graph";
        return false;
    }

    return true;
}

bool AudioEngine::initFilterGraph()
{
    if (m_filterGraph) {
        avfilter_graph_free(&m_filterGraph);
    }
    m_filterGraph = avfilter_graph_alloc();

    // ==================== BẮT ĐẦU CẢI TIẾN ÂM LƯỢNG ====================
    // Lấy thêm bộ lọc 'volume'
    const AVFilter *src = avfilter_get_by_name("abuffer");
    const AVFilter *atempo = avfilter_get_by_name("atempo");
    const AVFilter *volume = avfilter_get_by_name("volume");
    const AVFilter *sink = avfilter_get_by_name("abuffersink");
    // ===================== KẾT THÚC CẢI TIẾN ÂM LƯỢNG =====================

    AVStream *audioStream = m_formatCtx->streams[m_audioStreamIndex];
    QString args = QString("time_base=%1/%2:sample_rate=%3:sample_fmt=%4:channel_layout=%5")
                       .arg(audioStream->time_base.num).arg(audioStream->time_base.den)
                       .arg(m_codecCtx->sample_rate)
                       .arg(av_get_sample_fmt_name(m_codecCtx->sample_fmt))
                       .arg(m_codecCtx->ch_layout.nb_channels == 2 ? "stereo" : "mono");
    
    int ret = avfilter_graph_create_filter(&m_srcFilterCtx, src, "in", args.toStdString().c_str(), nullptr, m_filterGraph);
    if (ret < 0) return false;

    ret = avfilter_graph_create_filter(&m_sinkFilterCtx, sink, "out", nullptr, nullptr, m_filterGraph);
    if (ret < 0) return false;
    
    enum AVSampleFormat out_sample_fmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};
    av_opt_set_int_list(m_sinkFilterCtx, "sample_fmts", out_sample_fmts, -1, AV_OPT_SEARCH_CHILDREN);

    AVFilterContext *atempoCtx;
    QString atempo_args = QString::number(m_rate, 'f', 2);
    ret = avfilter_graph_create_filter(&atempoCtx, atempo, "atempo", atempo_args.toStdString().c_str(), nullptr, m_filterGraph);
    if (ret < 0) return false;

    // ==================== BẮT ĐẦU CẢI TIẾN ÂM LƯỢNG ====================
    // Tạo bộ lọc volume
    AVFilterContext *volumeCtx;
    QString volume_args = QString("volume=%1").arg(QString::number(m_softwareVolume, 'f', 2));
    ret = avfilter_graph_create_filter(&volumeCtx, volume, "volume", volume_args.toStdString().c_str(), nullptr, m_filterGraph);
    if (ret < 0) return false;

    // Liên kết các bộ lọc: src -> atempo -> volume -> sink
    if (avfilter_link(m_srcFilterCtx, 0, atempoCtx, 0) < 0 ||
        avfilter_link(atempoCtx, 0, volumeCtx, 0) < 0 ||
        avfilter_link(volumeCtx, 0, m_sinkFilterCtx, 0) < 0) {
        return false;
    }
    // ===================== KẾT THÚC CẢI TIẾN ÂM LƯỢNG =====================

    return avfilter_graph_config(m_filterGraph, nullptr) >= 0;
}

void AudioEngine::cleanup()
{
    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
        m_audioDevice = nullptr;
    }

    avfilter_graph_free(&m_filterGraph);
    avcodec_free_context(&m_codecCtx);
    avformat_close_input(&m_formatCtx);
    av_frame_free(&m_frame);
    av_packet_free(&m_packet);
}

void AudioEngine::run()
{
    if (!initialize(m_filePath)) {
        cleanup();
        emit stateChanged(State::Stopped);
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_state = State::Playing;
    }
    emit stateChanged(State::Playing);
    emit durationChanged(m_formatCtx->duration / (AV_TIME_BASE / 1000));

    AVFrame *filt_frame = av_frame_alloc();
    
    while (true) {
        if (m_fade_out_requested.load()) {
            if (!m_fadeTimer.isValid()) {
                m_fadeTimer.start();
                if (m_audioSink) m_volumeBeforeFade = m_audioSink->volume();
            }

            qint64 elapsed = m_fadeTimer.elapsed();
            if (elapsed >= FADE_DURATION_MS) {
                if (m_audioSink) m_audioSink->setVolume(0);
                QMutexLocker locker(&m_mutex);
                m_state = State::Stopped;
            } else {
                if (m_audioSink) {
                    qreal newVolume = m_volumeBeforeFade * (1.0 - (qreal)elapsed / FADE_DURATION_MS);
                    m_audioSink->setVolume(newVolume);
                }
            }
        }
        
        m_mutex.lock();
        State currentState = m_state;
        qint64 seekRequest = m_seekRequest;
        bool rateChanged = m_rateChanged;
        // ==================== BẮT ĐẦU CẢI TIẾN ÂM LƯỢNG ====================
        bool volumeChanged = m_volumeChanged;
        m_volumeChanged = false;
        // ===================== KẾT THÚC CẢI TIẾN ÂM LƯỢNG =====================
        m_seekRequest = -1;
        m_rateChanged = false;
        m_mutex.unlock();

        if (currentState == State::Stopped) {
            break;
        }

        if (seekRequest >= 0) {
            int64_t ts = seekRequest * m_formatCtx->streams[m_audioStreamIndex]->time_base.den / (1000 * m_formatCtx->streams[m_audioStreamIndex]->time_base.num);
            if (av_seek_frame(m_formatCtx, m_audioStreamIndex, ts, AVSEEK_FLAG_BACKWARD) >= 0) {
                 avcodec_flush_buffers(m_codecCtx);
                 initFilterGraph();
                 emit seekFinished();
            }
        }

        // ==================== BẮT ĐẦU CẢI TIẾN ÂM LƯỢNG ====================
        if (rateChanged || volumeChanged) {
            initFilterGraph();
        }
        // ===================== KẾT THÚC CẢI TIẾN ÂM LƯỢNG =====================

        PauseRequest pause_req = m_pause_request.exchange(PauseRequest::None);
        if (pause_req == PauseRequest::Suspend && m_audioSink) {
            m_audioSink->suspend();
        } else if (pause_req == PauseRequest::Resume && m_audioSink) {
            m_audioSink->resume();
        }
        
        if (currentState == State::Paused) {
            msleep(50);
            continue;
        }

        if (av_read_frame(m_formatCtx, m_packet) >= 0) {
            if (m_packet->stream_index == m_audioStreamIndex) {
                if (avcodec_send_packet(m_codecCtx, m_packet) == 0) {
                    while (avcodec_receive_frame(m_codecCtx, m_frame) == 0) {
                        if (av_buffersrc_add_frame_flags(m_srcFilterCtx, m_frame, AV_BUFFERSRC_FLAG_KEEP_REF) >= 0) {
                            while (av_buffersink_get_frame(m_sinkFilterCtx, filt_frame) >= 0) {
                                const int size = filt_frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)filt_frame->format) * filt_frame->ch_layout.nb_channels;
                                
                                while (m_audioSink && m_audioSink->bytesFree() < size) {
                                    QMutexLocker locker(&m_mutex);
                                    if (m_state != State::Playing) {
                                        goto end_of_frame_processing;
                                    }
                                    locker.unlock();
                                    msleep(10);
                                }

                                if (m_audioDevice) {
                                    m_audioDevice->write((const char*)filt_frame->data[0], size);
                                }
                                
                                qint64 pos = filt_frame->pts * 1000 * m_formatCtx->streams[m_audioStreamIndex]->time_base.num / m_formatCtx->streams[m_audioStreamIndex]->time_base.den;
                                
                                if (m_posUpdateTimer.elapsed() >= POSITION_UPDATE_INTERVAL_MS) {
                                    emit positionChanged(pos);
                                    m_posUpdateTimer.restart();
                                }

                                av_frame_unref(filt_frame);
                            }
                        }
                    }
                }
            }
            av_packet_unref(m_packet);
        } else {
            QMutexLocker locker(&m_mutex);
            m_state = State::Finished;
            locker.unlock();
            break;
        }
        end_of_frame_processing:;
    }

    State finalState;
    {
        QMutexLocker locker(&m_mutex);
        finalState = m_state;
    }

    av_frame_free(&filt_frame);
    cleanup();
    emit stateChanged(finalState);
}
