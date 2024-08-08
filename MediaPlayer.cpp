#include <QDebug>
#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(QObject *parent) :
    QObject(parent), m_player(new QMediaPlayer(this)), m_audioOutput(new QAudioOutput(this)),
    m_metadataManager(new MetadataManager(m_player, this))
{
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.5);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayer::playbackStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayer::onMediaStatusChanged);
    connect(m_metadataManager, &MetadataManager::metadataChanged, this, &MediaPlayer::metadataChanged);
}

QString MediaPlayer::source() const
{
    return m_source;
}

void MediaPlayer::setSource(const QString &source)
{
    if (m_source != source) {
        m_source = source;
        qDebug() << "Received source:" << source;
        QUrl url = QUrl::fromUserInput(source);
        if (!url.isValid()) {
            qWarning() << "Invalid URL:" << source;
            return;
        }
        qDebug() << "Setting source to:" << url.toString(QUrl::FullyEncoded);
        m_player->setSource(url);
        emit sourceChanged();
    }
}

QMediaPlayer::PlaybackState MediaPlayer::playbackState() const
{
    return m_player->playbackState();
}

QString MediaPlayer::title() const
{
    return m_metadataManager->title();
}

QString MediaPlayer::artist() const
{
    return m_metadataManager->artist();
}

QUrl MediaPlayer::coverArtUrl() const
{
    return m_metadataManager->coverArtUrl();
}

void MediaPlayer::playPause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        qDebug() << "Pause button clicked";
        m_player->pause();
    } else {
        qDebug() << "Play button clicked";
        m_player->play();
    }
}

void MediaPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "Media status changed:" << status;
    if (status == QMediaPlayer::LoadedMedia) {
        emit metadataChanged();
    }
}
