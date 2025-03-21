#include "mediaplayerwrapper.h"

#include <QAudioOutput>
#include <QTimer>

class MediaPlayerWrapperPrivate {
public:
    QMediaPlayer m_player;
    QAudioOutput m_audioOutput;

    qint64 m_savedPosition = 0.0;
    qint64 m_undoSavedPosition = 0.0;
    bool m_hasSavedPosition = false;

    QMediaPlayer::PlaybackState m_playbackState = m_player.playbackState();
    QMediaPlayer::MediaStatus m_status = m_player.mediaStatus();
    bool m_queuedStatusUpdates = false; // Thread safe status updates
};

MediaPlayerWrapper::MediaPlayerWrapper(QObject* parent)
    : QObject(parent), mp(std::make_unique<MediaPlayerWrapperPrivate>()) {
    mp->m_player.setAudioOutput(&mp->m_audioOutput);

    // clang-format off
    connect(&mp->m_player, &QMediaPlayer::sourceChanged, this, [this] {
        QTimer::singleShot(0, [this] { Q_EMIT sourceChanged(); });
    });
    connect(&mp->m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayerWrapper::queueStatusChanged);
    connect(&mp->m_player, QOverload<QMediaPlayer::Error, const QString &>::of(&QMediaPlayer::errorOccurred), this, [this](const QMediaPlayer::Error error, const QString &errorString) {
        qDebug() << "Error occurred in QMediaPlayer: " << errorString;
        Q_EMIT errorChanged(error);
    });
    connect(&mp->m_audioOutput, &QAudioOutput::volumeChanged, this, &MediaPlayerWrapper::trackVolumeChanged);
    connect(&mp->m_audioOutput, &QAudioOutput::mutedChanged, this, &MediaPlayerWrapper::trackMutedChanged);
    connect(&mp->m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayerWrapper::queueStatusChanged);
    connect(&mp->m_player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        QTimer::singleShot(0, [this, duration] { Q_EMIT durationChanged(duration); });
    });
    connect(&mp->m_player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        QTimer::singleShot(0, [this, position] { Q_EMIT positionChanged(position); });
    });
    connect(&mp->m_player, &QMediaPlayer::seekableChanged, this, [this](bool seekable) {
        QTimer::singleShot(0, [this, seekable] { Q_EMIT seekableChanged(seekable); });
    });
    // clang-format on
}

MediaPlayerWrapper::~MediaPlayerWrapper() = default;

QUrl MediaPlayerWrapper::source() const {
    return mp->m_player.source();
}

QMediaPlayer::MediaStatus MediaPlayerWrapper::status() const {
    return mp->m_status;
}

QMediaPlayer::PlaybackState MediaPlayerWrapper::playbackState() const {
    return mp->m_playbackState;
}

float MediaPlayerWrapper::volume() const {
    /**
     *  We should convert between linear for audio output and logarithmic for human ear
     *  https://doc.qt.io/qt-6/qtaudio.html
     */
    const float linearVolume = mp->m_audioOutput.volume();
    const float logarithmicVolume =
        QtAudio::convertVolume(linearVolume, QtAudio::LinearVolumeScale, QtAudio::LogarithmicVolumeScale);

    return logarithmicVolume * 100;
}

bool MediaPlayerWrapper::isMuted() const {
    return mp->m_audioOutput.isMuted();
}

QMediaPlayer::Error MediaPlayerWrapper::error() const {
    return mp->m_player.error();
}

qint64 MediaPlayerWrapper::duration() const {
    return mp->m_player.duration();
}

qint64 MediaPlayerWrapper::position() const {
    return mp->m_player.position();
}

bool MediaPlayerWrapper::isSeekable() const {
    return mp->m_player.isSeekable();
}

void MediaPlayerWrapper::setSource(const QUrl& newSource) const {
    if (mp->m_player.mediaStatus() == QMediaPlayer::EndOfMedia && mp->m_player.source() == newSource) {
        mp->m_player.setPosition(0);
    } else {
        mp->m_player.setSource(newSource);
    }
}

void MediaPlayerWrapper::play() const {
    mp->m_player.play();

    if (mp->m_hasSavedPosition) {
        mp->m_player.setPosition(mp->m_savedPosition);
        mp->m_hasSavedPosition = false;
    }
}

void MediaPlayerWrapper::pause() const {
    mp->m_player.pause();
}

void MediaPlayerWrapper::stop() const {
    mp->m_player.stop();
}

void MediaPlayerWrapper::setVolume(const float newVolume) const {
    const qreal linearVolume = QtAudio::convertVolume(newVolume / static_cast<qreal>(100),
                                                      QtAudio::LogarithmicVolumeScale, QtAudio::LinearVolumeScale);

    mp->m_audioOutput.setVolume(linearVolume);
}

void MediaPlayerWrapper::setMuted(const bool newIsMuted) const {
    mp->m_audioOutput.setMuted(newIsMuted);
}

void MediaPlayerWrapper::setPosition(const qint64 newPosition) const {
    if (mp->m_player.duration() <= 0) {
        savePosition(newPosition);
        return;
    }
    mp->m_player.setPosition(newPosition);
}

void MediaPlayerWrapper::saveUndoPosition(const qint64 position) const {
    mp->m_undoSavedPosition = position;
}

void MediaPlayerWrapper::restoreUndoPosition() const {
    mp->m_hasSavedPosition = true;
    mp->m_savedPosition = mp->m_undoSavedPosition;
}

void MediaPlayerWrapper::seek(const qint64 newPosition) const {
    mp->m_player.setPosition(newPosition);
}

void MediaPlayerWrapper::trackStateChanged() {
    switch (mp->m_player.playbackState()) {
    case QMediaPlayer::PlaybackState::StoppedState:
        Q_EMIT stopped();
        break;
    case QMediaPlayer::PlaybackState::PlayingState:
        Q_EMIT playing();
        break;
    case QMediaPlayer::PlaybackState::PausedState:
        Q_EMIT paused();
        break;
    }
}

void MediaPlayerWrapper::trackVolumeChanged() {
    QTimer::singleShot(0, [this]() {
        Q_EMIT volumeChanged();
    });
}

void MediaPlayerWrapper::trackMutedChanged() {
    QTimer::singleShot(0, [this]() {
        Q_EMIT mutedChanged(isMuted());
    });
}

void MediaPlayerWrapper::savePosition(const qint64 position) const {
    if (!mp->m_hasSavedPosition) {
        mp->m_hasSavedPosition = true;
        mp->m_savedPosition = position;
    }
}

void MediaPlayerWrapper::notifyStatusChanges() {
    mp->m_queuedStatusUpdates = false;

    if (mp->m_player.mediaStatus() != mp->m_status) {
        mp->m_status = mp->m_player.mediaStatus();
        Q_EMIT statusChanged(mp->m_status);
    }

    if (mp->m_player.playbackState() != mp->m_playbackState) {
        mp->m_playbackState = mp->m_player.playbackState();
        Q_EMIT playbackStateChanged(mp->m_playbackState);
        trackStateChanged();
    }
}

void MediaPlayerWrapper::queueStatusChanged() {
    if (!mp->m_queuedStatusUpdates) {
        QTimer::singleShot(0, this, &MediaPlayerWrapper::notifyStatusChanges);
        mp->m_queuedStatusUpdates = true;
    }
}
