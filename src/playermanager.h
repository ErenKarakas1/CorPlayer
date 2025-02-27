#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <QPersistentModelIndex>
#include <QQmlEngine>

class PlayerManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // clang-format off
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(bool canPlay READ canPlay NOTIFY canPlayChanged)
    Q_PROPERTY(bool canSkipBackward READ canSkipBackward NOTIFY canSkipBackwardChanged)
    Q_PROPERTY(bool canSkipForward READ canSkipForward NOTIFY canSkipForwardChanged)

    Q_PROPERTY(QPersistentModelIndex currentTrack READ currentTrack WRITE setCurrentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QPersistentModelIndex previousTrack READ previousTrack WRITE setPreviousTrack NOTIFY previousTrackChanged)
    Q_PROPERTY(QPersistentModelIndex nextTrack READ nextTrack WRITE setNextTrack NOTIFY nextTrackChanged)
    // clang-format on

public:
    explicit PlayerManager(QObject *parent = nullptr);

    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] bool canPlay() const;
    [[nodiscard]] bool canSkipBackward() const;
    [[nodiscard]] bool canSkipForward() const;

    [[nodiscard]] QPersistentModelIndex currentTrack() const;
    [[nodiscard]] QPersistentModelIndex previousTrack() const;
    [[nodiscard]] QPersistentModelIndex nextTrack() const;

Q_SIGNALS:
    void isPlayingChanged();
    void canPlayChanged();
    void canSkipBackwardChanged();
    void canSkipForwardChanged();

    void currentTrackChanged();
    void previousTrackChanged();
    void nextTrackChanged();

public Q_SLOTS:
    void playing();
    void pausedOrStopped();

    void setCurrentTrack(const QPersistentModelIndex &currentTrack);
    void setPreviousTrack(const QPersistentModelIndex &previousTrack);
    void setNextTrack(const QPersistentModelIndex &nextTrack);

private:
    QPersistentModelIndex m_previousTrack;
    QPersistentModelIndex m_currentTrack;
    QPersistentModelIndex m_nextTrack;
    bool m_isInPlayingState = false;
};

#endif // PLAYERMANAGER_H
