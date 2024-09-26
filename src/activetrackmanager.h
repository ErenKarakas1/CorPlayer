#ifndef ACTIVETRACKMANAGER_H
#define ACTIVETRACKMANAGER_H

#include "playerutils.h"

#include <QMediaPlayer>
#include <QObject>
#include <QPersistentModelIndex>
#include <QQmlEngine>
#include <QUrl>

class QDateTime;

class ActiveTrackManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // clang-format off
    Q_PROPERTY(QPersistentModelIndex currentTrack READ currentTrack WRITE setCurrentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QAbstractItemModel *playlistModel READ playlistModel WRITE setPlaylistModel NOTIFY playlistModelChanged)

    Q_PROPERTY(QUrl trackSource READ trackSource NOTIFY trackSourceChanged)
    Q_PROPERTY(int titleRole READ titleRole WRITE setTitleRole NOTIFY titleRoleChanged)
    Q_PROPERTY(int artistRole READ artistRole WRITE setArtistRole NOTIFY artistRoleChanged)
    Q_PROPERTY(int albumRole READ albumRole WRITE setAlbumRole NOTIFY albumRoleChanged)
    Q_PROPERTY(int urlRole READ urlRole WRITE setUrlRole NOTIFY urlRoleChanged)
    Q_PROPERTY(int isPlayingRole READ isPlayingRole WRITE setIsPlayingRole NOTIFY isPlayingRoleChanged)
    Q_PROPERTY(QMediaPlayer::MediaStatus trackStatus READ trackStatus WRITE setTrackStatus NOTIFY trackStatusChanged)

    Q_PROPERTY(QMediaPlayer::PlaybackState trackPlaybackState
        READ trackPlaybackState WRITE setTrackPlaybackState NOTIFY trackPlaybackStateChanged)

    Q_PROPERTY(QMediaPlayer::Error trackError READ trackError WRITE setTrackError NOTIFY trackErrorChanged)
    Q_PROPERTY(qint64 trackDuration READ trackDuration WRITE setTrackDuration NOTIFY trackDurationChanged)
    Q_PROPERTY(bool trackIsSeekable READ trackIsSeekable WRITE setTrackIsSeekable NOTIFY trackIsSeekableChanged)
    Q_PROPERTY(qint64 trackPosition READ trackPosition WRITE setTrackPosition NOTIFY trackPositionChanged)

    Q_PROPERTY(qint64 trackControlPosition
        READ trackControlPosition WRITE setTrackControlPosition NOTIFY trackControlPositionChanged)

    Q_PROPERTY(QVariantMap persistentState READ persistentState WRITE setPersistentState NOTIFY persistentStateChanged)
    // clang-format on

public:
    explicit ActiveTrackManager(QObject *parent = nullptr);
    [[nodiscard]] QPersistentModelIndex currentTrack() const;
    [[nodiscard]] QAbstractItemModel *playlistModel() const;
    [[nodiscard]] QUrl trackSource() const;
    [[nodiscard]] int titleRole() const;
    [[nodiscard]] int artistRole() const;
    [[nodiscard]] int albumRole() const;
    [[nodiscard]] int urlRole() const;
    [[nodiscard]] int isPlayingRole() const;
    [[nodiscard]] QMediaPlayer::MediaStatus trackStatus() const;
    [[nodiscard]] QMediaPlayer::PlaybackState trackPlaybackState() const;
    [[nodiscard]] QMediaPlayer::Error trackError() const;
    [[nodiscard]] qint64 trackDuration() const;
    [[nodiscard]] bool trackIsSeekable() const;
    [[nodiscard]] qint64 trackPosition() const;
    [[nodiscard]] qint64 trackControlPosition() const;
    [[nodiscard]] QVariantMap persistentState() const;

Q_SIGNALS:
    void currentTrackChanged();
    void playlistModelChanged();
    void trackSourceChanged(const QUrl &trackSource);
    void titleRoleChanged();
    void artistRoleChanged();
    void albumRoleChanged();
    void urlRoleChanged();
    void isPlayingRoleChanged();
    void trackStatusChanged();
    void trackPlaybackStateChanged();
    void trackErrorChanged();
    void trackDurationChanged();
    void trackIsSeekableChanged();
    void trackPositionChanged();
    void trackControlPositionChanged();
    void persistentStateChanged();

    void trackPlay();
    void trackPause();
    void trackStop();
    void skipNextTrack(PlayerUtils::SkipReason reason = PlayerUtils::SkipReason::Automatic);

    void seek(qint64 position);
    void saveUndoPositionInWrapper(qint64 position);
    void restoreUndoPositionInWrapper();
    void sourceInError(const QUrl &source, QMediaPlayer::Error trackError);
    void displayTrackError(const QString &fileName);
    void trackStartedPlaying(const QUrl &fileName, const QDateTime &time);
    void trackFinishedPlaying(const QUrl &fileName, const QDateTime &time);
    void updateData(const QPersistentModelIndex &index, const QVariant &value, int role);

public Q_SLOTS:
    void setCurrentTrack(const QPersistentModelIndex &newCurrentTrack);
    void saveForUndoClearPlaylist();
    void restoreForUndoClearPlaylist();
    void setPlaylistModel(QAbstractItemModel *newPlaylistModel);

    void setTitleRole(int newTitleRole);
    void setArtistRole(int newArtistRole);
    void setAlbumRole(int newAlbumRole);
    void setUrlRole(int newUrlRole);
    void setIsPlayingRole(int newIsPlayingRole);
    void setTrackStatus(QMediaPlayer::MediaStatus newTrackStatus);
    void setTrackPlaybackState(QMediaPlayer::PlaybackState newTrackPlaybackState);
    void setTrackError(QMediaPlayer::Error newTrackError);
    void setTrackDuration(qint64 newTrackDuration);
    void setTrackIsSeekable(bool newTrackIsSeekable);
    void setTrackPosition(qint64 newTrackPosition);
    void setTrackControlPosition(int newTrackControlPosition);
    void setPersistentState(const QVariantMap &newPersistentState);

    void ensurePause();
    void ensurePlay();
    void requestPlay();
    void playPause();
    void stop();

    void trackSeek(int newPosition);
    void playlistFinished();
    void tracksDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    void notifyTrackSourceProperty();
    void triggerPlay();
    void triggerPause();
    void triggerStop();
    void triggerSkipNextTrack(PlayerUtils::SkipReason reason = PlayerUtils::SkipReason::Automatic);
    void restorePreviousState();

    QPersistentModelIndex m_currentTrack;
    QPersistentModelIndex m_previousTrack;
    QAbstractItemModel *m_playlistModel = nullptr;

    int m_titleRole = Qt::DisplayRole;
    int m_artistRole = Qt::DisplayRole;
    int m_albumRole = Qt::DisplayRole;
    int m_urlRole = Qt::DisplayRole;
    int m_isPlayingRole = Qt::DisplayRole;

    QVariant m_previousTrackSource;
    QMediaPlayer::MediaStatus m_trackStatus = QMediaPlayer::NoMedia;
    QMediaPlayer::PlaybackState m_trackPlaybackState = QMediaPlayer::StoppedState;
    QMediaPlayer::Error m_trackError = QMediaPlayer::NoError;

    bool m_isPlaying = false;
    bool m_skippingCurrentTrack = false;
    qint64 m_trackDuration = 0;
    bool m_trackIsSeekable = false;
    qint64 m_trackPosition = 0;
    QVariantMap m_persistentState;
    bool m_undoPlayingState = false;
    qint64 m_undoTrackPosition = 0;
};

#endif // ACTIVETRACKMANAGER_H
