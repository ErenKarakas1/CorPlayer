#ifndef MEDIAPLAYERWRAPPER_H
#define MEDIAPLAYERWRAPPER_H

#include <QObject>
#include <QQmlEngine>
#include <QMediaPlayer>
#include <QAudioOutput>

#include <memory>

class MediaPlayerWrapperPrivate;

class MediaPlayerWrapper : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QMediaPlayer::MediaStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QMediaPlayer::PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool isMuted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool isSeekable READ isSeekable NOTIFY seekableChanged)

public:
    explicit MediaPlayerWrapper(QObject* parent = nullptr);
    ~MediaPlayerWrapper() override;
    [[nodiscard]] QUrl source() const;
    [[nodiscard]] QMediaPlayer::MediaStatus status() const;
    [[nodiscard]] QMediaPlayer::PlaybackState playbackState() const;
    [[nodiscard]] qreal volume() const;
    [[nodiscard]] bool isMuted() const;
    [[nodiscard]] qint64 duration() const;
    [[nodiscard]] qint64 position() const;
    [[nodiscard]] bool isSeekable() const;

Q_SIGNALS:
    void sourceChanged();
    void statusChanged(QMediaPlayer::MediaStatus status);
    void playbackStateChanged(QMediaPlayer::PlaybackState playbackState);
    void volumeChanged();
    void mutedChanged(bool isMuted);
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void seekableChanged(bool isSeekable);

public Q_SLOTS:
    void setSource(const QUrl& newSource);
    void play();
    void pause();
    void stop();
    void setVolume(qreal newVolume);
    void setMuted(bool newIsMuted);
    void setPosition(qint64 newPosition);
    void saveUndoPosition(qint64 position);
    void restoreUndoPosition();
    void seek(qint64 newPosition);

private Q_SLOTS:
    void trackStateChanged();
    void trackVolumeChanged();
    void trackMutedChanged();

private:
    void savePosition(qint64 position);

    friend class MediaPlayerWrapperPrivate;
    std::unique_ptr<MediaPlayerWrapperPrivate> mp;
};

#endif // MEDIAPLAYERWRAPPER_H
