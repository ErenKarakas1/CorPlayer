#include "MediaPlayer.h"
#include <QDebug>

MediaPlayer::MediaPlayer(QObject *parent)
    : QObject(parent), m_mediaPlayer(new QMediaPlayer(this)), m_playing(false)
{
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        setPlaying(state == QMediaPlayer::PlayingState);
    });
}

QString MediaPlayer::source() const
{
    return m_source;
}

void MediaPlayer::setSource(const QString &source)
{
    if (m_source == source)
        return;

    m_source = source;
    m_mediaPlayer->setSource(QUrl::fromLocalFile(source));
    qDebug() << "Media source set to:" << source;
    emit sourceChanged();
}

bool MediaPlayer::isPlaying() const
{
    return m_playing;
}

void MediaPlayer::setPlaying(bool playing)
{
    if (m_playing == playing)
        return;

    m_playing = playing;
    if (playing)
        m_mediaPlayer->play();
    else
        m_mediaPlayer->pause();

    emit playingChanged();
}
