#include "mediaplayerwrapper.h"

#include <QDebug>
#include <QTimer>

class MediaPlayerWrapperPrivate {
public:
    QMediaPlayer m_player;
    QAudioOutput m_audioOutput;

    qint64 m_savedPosition = 0.0;
    qint64 m_undoSavedPosition = 0.0;
    bool m_hasSavedPosition = false;

    QMediaPlayer::PlaybackState m_playbackState = m_player.playbackState();
    QMediaPlayer::MediaStatus m_status = m_player.mediaStatus();;
    bool m_queuedStatusUpdates = false; // Thread safe status updates
};

MediaPlayerWrapper::MediaPlayerWrapper(QObject *parent) :
    QObject(parent), mp(std::make_unique<MediaPlayerWrapperPrivate>()) {
    mp->m_player.setAudioOutput(&mp->m_audioOutput);

    connect(&mp->m_player, &QMediaPlayer::sourceChanged, this, &MediaPlayerWrapper::sourceChanged);
    connect(&mp->m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayerWrapper::queueStatusChanged);
    connect(&mp->m_player, QOverload<QMediaPlayer::Error, const QString &>::of(&QMediaPlayer::errorOccurred), this, &MediaPlayerWrapper::errorChanged);
    connect(&mp->m_audioOutput, &QAudioOutput::volumeChanged, this, &MediaPlayerWrapper::trackVolumeChanged);
    connect(&mp->m_audioOutput, &QAudioOutput::mutedChanged, this, &MediaPlayerWrapper::trackMutedChanged);
    connect(&mp->m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayerWrapper::queueStatusChanged);
    connect(&mp->m_player, &QMediaPlayer::durationChanged, this, &MediaPlayerWrapper::durationChanged);
    connect(&mp->m_player, &QMediaPlayer::positionChanged, this, &MediaPlayerWrapper::positionChanged);
    connect(&mp->m_player, &QMediaPlayer::seekableChanged, this, &MediaPlayerWrapper::seekableChanged);
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

qreal MediaPlayerWrapper::volume() const {
    qDebug() << mp->m_audioOutput.volume() * 100;
    return mp->m_audioOutput.volume() * 100;
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

void MediaPlayerWrapper::setSource(const QUrl &source) {
    mp->m_player.setSource(source);
}

void MediaPlayerWrapper::play() {
    if (mp->m_hasSavedPosition) {
        mp->m_player.setPosition(mp->m_savedPosition);
        mp->m_hasSavedPosition = false;
    }

    mp->m_player.play();
}

void MediaPlayerWrapper::pause() {
    mp->m_player.pause();
}

void MediaPlayerWrapper::stop() {
    mp->m_player.stop();
}

void MediaPlayerWrapper::setVolume(qreal volume) {
    mp->m_audioOutput.setVolume(volume / 100);
}

void MediaPlayerWrapper::setMuted(bool isMuted) {
    mp->m_audioOutput.setMuted(isMuted);
}

void MediaPlayerWrapper::setPosition(qint64 position) {
    mp->m_player.setPosition(position);
}

void MediaPlayerWrapper::saveUndoPosition(qint64 position) {
    mp->m_undoSavedPosition = position;
}

void MediaPlayerWrapper::restoreUndoPosition() {
    mp->m_hasSavedPosition = true;
    mp->m_savedPosition = mp->m_undoSavedPosition;
}

void MediaPlayerWrapper::seek(qint64 position) {
    mp->m_player.setPosition(position);
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

void MediaPlayerWrapper::savePosition(qint64 position) {
    if (!mp->m_hasSavedPosition) {
        mp->m_hasSavedPosition = true;
        mp->m_savedPosition = position;
    }
}

/**
 * We should use QMetaObject::invokeMethod and Qt::QueuedConnection to emit signals in the main thread
 * Direct access is discouraged
 * https://stackoverflow.com/questions/45696232/qmediaplayer-not-loading-media-and-not-emitting-mediastatuschanged-signals
 */

void MediaPlayerWrapper::trackStateSignalChanges(QMediaPlayer::PlaybackState newState) {
    QMetaObject::invokeMethod(this, [this, newState]() {
        Q_EMIT playbackStateChanged(newState);
    }, Qt::QueuedConnection);
}

void MediaPlayerWrapper::trackStatusSignalChanges(QMediaPlayer::MediaStatus newStatus) {
    QMetaObject::invokeMethod(this, [this, newStatus]() {
        Q_EMIT statusChanged(newStatus);
    }, Qt::QueuedConnection);
}

void MediaPlayerWrapper::trackDurationSignalChanges(qint64 newDuration) {
    QMetaObject::invokeMethod(this, [this, newDuration]() {
        Q_EMIT durationChanged(newDuration);
    }, Qt::QueuedConnection);
}

void MediaPlayerWrapper::trackPositionSignalChanges(qint64 newPosition) {
    QMetaObject::invokeMethod(this, [this, newPosition]() {
        Q_EMIT positionChanged(newPosition);
    }, Qt::QueuedConnection);
}

void MediaPlayerWrapper::trackSeekableSignalChanges(bool isSeekable) {
    QMetaObject::invokeMethod(this, [this, isSeekable]() {
        Q_EMIT seekableChanged(isSeekable);
    }, Qt::QueuedConnection);
}

void MediaPlayerWrapper::trackVolumeSignalChanges() {
    QMetaObject::invokeMethod(this, [this]() {
        Q_EMIT volumeChanged();
    }, Qt::QueuedConnection);
}

void MediaPlayerWrapper::trackMutedSignalChanges(bool isMuted) {
    QMetaObject::invokeMethod(this, [this, isMuted]() {
        Q_EMIT mutedChanged(isMuted);
    }, Qt::QueuedConnection);
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
