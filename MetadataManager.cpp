#include <QDebug>
#include <QImage>
#include <QMediaMetaData>
#include <QDateTime>
#include "MetadataManager.h"

MetadataManager::MetadataManager(QMediaPlayer *player, QObject *parent)
    : QObject(parent), m_player(player)
{
    connect(m_player, &QMediaPlayer::metaDataChanged, this, &MetadataManager::updateMetadata);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MetadataManager::handleMediaStatusChanged);
}

QString MetadataManager::title() const
{
    return m_title;
}

QString MetadataManager::artist() const
{
    return m_artist;
}

QUrl MetadataManager::coverArtUrl() const
{
    return m_coverArtUrl;
}

void MetadataManager::updateMetadata()
{
    m_title = m_player->metaData().stringValue(QMediaMetaData::Title);
    m_artist = m_player->metaData().stringValue(QMediaMetaData::ContributingArtist);
    QVariant coverArt = m_player->metaData().value(QMediaMetaData::ThumbnailImage);
    if (coverArt.isValid()) {
        QImage image = coverArt.value<QImage>();
        QImage resizedImage = image.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QString filePath = QString("/tmp/coverart_%1.jpg").arg(QDateTime::currentMSecsSinceEpoch()); // TODO /tmp probably linux only
        if (resizedImage.save(filePath)) {
            m_coverArtUrl = QUrl::fromLocalFile(filePath);
        } else {
            m_coverArtUrl = QUrl();
            qDebug() << "Failed to save cover art to file";
        }
    } else {
        m_coverArtUrl = QUrl();
        qDebug() << "No cover art available";
    }

    qDebug() << "Metadata updated - Title:" << m_title << ", Artist:" << m_artist << ", Cover Art URL:" << m_coverArtUrl;
    emit metadataChanged();
}

void MetadataManager::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia) {
        updateMetadata();
    }
}
