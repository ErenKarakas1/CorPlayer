#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "MetadataManager.h"

class MediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QMediaPlayer::PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(QString title READ title NOTIFY metadataChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY metadataChanged)
    Q_PROPERTY(QUrl coverArtUrl READ coverArtUrl NOTIFY metadataChanged)

public:
    explicit MediaPlayer(QObject *parent = nullptr);

    QString source() const;
    void setSource(const QString &source);
    QMediaPlayer::PlaybackState playbackState() const;
    QString title() const;
    QString artist() const;
    QUrl coverArtUrl() const;

public slots:
    void playPause();

signals:
    void sourceChanged();
    void playbackStateChanged();
    void metadataChanged();

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    MetadataManager *m_metadataManager;
    QString m_source;
};

#endif // MEDIAPLAYER_H
