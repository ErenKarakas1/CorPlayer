#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QUrl>

class MetadataManager : public QObject
{
    Q_OBJECT

public:
    MetadataManager(QMediaPlayer *player, QObject *parent = nullptr);

    QString title() const;
    QString artist() const;
    QUrl coverArtUrl() const;

signals:
    void metadataChanged();

private slots:
    void updateMetadata();
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QMediaPlayer *m_player;
    QString m_title;
    QString m_artist;
    QUrl m_coverArtUrl;
};

#endif // METADATAMANAGER_H
