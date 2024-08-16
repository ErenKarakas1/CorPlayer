#include "mediaplayerwrapper.h"

#include <QDebug>

class MediaPlayerWrapperPrivate {
public:
    QMediaPlayer m_player;
    QAudioOutput m_audioOutput;

    qint64 m_savedPosition = 0.0;
    qint64 m_undoSavedPosition = 0.0;
    bool m_hasSavedPosition = false;

    QMediaPlayer::PlaybackState m_playbackState = m_player.playbackState();
    QMediaPlayer::MediaStatus m_status = m_player.mediaStatus();;
};

MediaPlayerWrapper::MediaPlayerWrapper(QObject *parent) :
    QObject(parent), mp(std::make_unique<MediaPlayerWrapperPrivate>()) {
    mp->m_player.setAudioOutput(&mp->m_audioOutput);

    connect(&mp->m_player, &QMediaPlayer::sourceChanged, this, &MediaPlayerWrapper::sourceChanged);
    connect(&mp->m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayerWrapper::trackStateChanged);
    connect(&mp->m_audioOutput, &QAudioOutput::volumeChanged, this, &MediaPlayerWrapper::trackVolumeChanged);
    connect(&mp->m_audioOutput, &QAudioOutput::mutedChanged, this, &MediaPlayerWrapper::trackMutedChanged);
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

qint64 MediaPlayerWrapper::duration() const {
    return mp->m_player.duration();
}

qint64 MediaPlayerWrapper::position() const {
    return mp->m_player.position();
}

bool MediaPlayerWrapper::isSeekable() const {
    return mp->m_player.isSeekable();
}

void MediaPlayerWrapper::setSource(const QUrl& source) {
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
    // TODO should use different signals for play, stop, pause?
    mp->m_playbackState = mp->m_player.playbackState();
    Q_EMIT playbackStateChanged(mp->m_playbackState);
}

void MediaPlayerWrapper::trackVolumeChanged() {
    Q_EMIT volumeChanged();
}

void MediaPlayerWrapper::trackMutedChanged() {
    Q_EMIT mutedChanged(mp->m_audioOutput.isMuted());
}

void MediaPlayerWrapper::savePosition(qint64 position) {
    if (!mp->m_hasSavedPosition) {
        mp->m_hasSavedPosition = true;
        mp->m_savedPosition = position;
    }
}
