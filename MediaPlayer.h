#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QMediaPlayer>

class MediaPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool playing READ isPlaying WRITE setPlaying NOTIFY playingChanged)

public:
    explicit MediaPlayer(QObject *parent = nullptr);

    QString source() const;
    void setSource(const QString &source);

    bool isPlaying() const;
    void setPlaying(bool playing);

signals:
    void sourceChanged();
    void playingChanged();

private:
    QMediaPlayer *m_mediaPlayer;
    QString m_source;
    bool m_playing;
};

#endif // MEDIAPLAYER_H
