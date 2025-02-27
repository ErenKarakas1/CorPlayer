#ifndef ACTIVETRACKMANAGER_H
#define ACTIVETRACKMANAGER_H

#include "playerutils.hpp"

#include <QMediaPlayer>
#include <QObject>
#include <QPersistentModelIndex>
#include <QQmlEngine>
#include <QTimer>
#include <QUrl>

class TrackMetadata : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY artistChanged)
    Q_PROPERTY(QString album READ album NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ albumArtist NOTIFY albumArtistChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl coverUrl READ coverUrl NOTIFY coverUrlChanged)
    Q_PROPERTY(quint64 databaseId READ databaseId NOTIFY databaseIdChanged)
    Q_PROPERTY(PlayerUtils::PlaylistEntryType elementType READ elementType NOTIFY elementTypeChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(quint64 albumId READ albumId NOTIFY albumIdChanged)
    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)

public:
    explicit TrackMetadata(QObject* parent = nullptr);

    [[nodiscard]] QString title() const;
    [[nodiscard]] QString artist() const;
    [[nodiscard]] QString album() const;
    [[nodiscard]] QString albumArtist() const;
    [[nodiscard]] QUrl fileUrl() const;
    [[nodiscard]] QUrl coverUrl() const;
    [[nodiscard]] quint64 databaseId() const;
    [[nodiscard]] PlayerUtils::PlaylistEntryType elementType() const;
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] quint64 albumId() const;
    [[nodiscard]] bool isValid() const;

    void setRoles(int titleRole, int artistRole, int albumRole, int albumArtistRole, int fileUrlRole, int coverUrlRole,
                  int databaseIdRole, int elementTypeRole, int isPlayingRole, int albumIdRole, int isValidRole);

public Q_SLOTS:
    void setCurrentTrack(const QPersistentModelIndex& currentTrack);
    void updateMetadata();

Q_SIGNALS:
    void titleChanged();
    void artistChanged();
    void albumChanged();
    void albumArtistChanged();
    void fileUrlChanged();
    void coverUrlChanged();
    void databaseIdChanged();
    void elementTypeChanged();
    void isPlayingChanged();
    void albumIdChanged();
    void isValidChanged();

protected:
    struct Roles {
        int title = Qt::DisplayRole;
        int artist = Qt::DisplayRole;
        int album = Qt::DisplayRole;
        int albumArtist = Qt::DisplayRole;
        int fileUrl = Qt::DisplayRole;
        int coverUrl = Qt::DisplayRole;
        int databaseId = Qt::DisplayRole;
        int elementType = Qt::DisplayRole;
        int isPlaying = Qt::DisplayRole;
        int albumId = Qt::DisplayRole;
        int isValid = Qt::DisplayRole;
    } m_roles;

private:
    friend class ActiveTrackManager;
    QPersistentModelIndex m_currentTrack;

    struct Metadata {
        QString title;
        QString artist;
        QString album;
        QString albumArtist;
        QUrl fileUrl;
        QUrl coverUrl;
        quint64 databaseId = 0;
        PlayerUtils::PlaylistEntryType elementType = PlayerUtils::PlaylistEntryType::Unknown;
        bool isPlaying = false;
        quint64 albumId = 0;
        bool isValid = false;
    } m_metadata;
};

class QDateTime;

class ActiveTrackManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // clang-format off
    Q_PROPERTY(TrackMetadata* trackMetadata READ trackMetadata CONSTANT)
    Q_PROPERTY(QPersistentModelIndex currentTrack READ currentTrack WRITE setCurrentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QAbstractItemModel* playlistModel READ playlistModel WRITE setPlaylistModel NOTIFY playlistModelChanged)

    Q_PROPERTY(QUrl trackSource READ trackSource NOTIFY trackSourceChanged)
    Q_PROPERTY(QMediaPlayer::MediaStatus mediaStatus READ mediaStatus WRITE setMediaStatus NOTIFY mediaStatusChanged)
    Q_PROPERTY(QMediaPlayer::PlaybackState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged)

    Q_PROPERTY(QMediaPlayer::Error trackError READ trackError WRITE setTrackError NOTIFY trackErrorChanged)
    Q_PROPERTY(qint64 duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool seekable READ seekable WRITE setSeekable NOTIFY seekableChanged)

    Q_PROPERTY(qint64 trackControlPosition READ trackControlPosition WRITE setTrackControlPosition NOTIFY trackControlPositionChanged)
    Q_PROPERTY(QVariantMap persistentState READ persistentState WRITE setPersistentState NOTIFY persistentStateChanged)
    // clang-format on

public:
    explicit ActiveTrackManager(QObject* parent = nullptr);

    [[nodiscard]] TrackMetadata* trackMetadata() const;
    [[nodiscard]] QPersistentModelIndex currentTrack() const;
    [[nodiscard]] QAbstractItemModel* playlistModel() const;

    [[nodiscard]] QUrl trackSource() const;
    [[nodiscard]] QMediaPlayer::MediaStatus mediaStatus() const;
    [[nodiscard]] QMediaPlayer::PlaybackState playbackState() const;
    [[nodiscard]] QMediaPlayer::Error trackError() const;

    [[nodiscard]] qint64 duration() const;
    [[nodiscard]] qint64 position() const;
    [[nodiscard]] bool seekable() const;

    [[nodiscard]] qint64 trackControlPosition() const;
    [[nodiscard]] QVariantMap persistentState() const;

Q_SIGNALS:
    void currentTrackChanged();
    void playlistModelChanged();

    void trackSourceChanged(const QUrl& trackSource);
    void mediaStatusChanged();
    void playbackStateChanged();
    void trackErrorChanged();

    void durationChanged();
    void positionChanged();
    void seekableChanged();

    void trackControlPositionChanged();
    void persistentStateChanged();

    void playTrack();
    void pauseTrack();
    void stopTrack();
    void skipNextTrack(PlayerUtils::SkipReason reason = PlayerUtils::SkipReason::Automatic);

    void seek(qint64 position);
    void saveUndoPositionInWrapper(qint64 position);
    void restoreUndoPositionInWrapper();
    void sourceInError(const QUrl& source, QMediaPlayer::Error trackError);
    void displayTrackError(const QString& fileName);
    void trackStartedPlaying(const QUrl& fileName, const QDateTime& time);
    void trackFinishedPlaying(const QUrl& fileName, const QDateTime& time);
    void updateData(const QPersistentModelIndex& index, const QVariant& value, int role);

public Q_SLOTS:
    void setCurrentTrack(const QPersistentModelIndex& currentTrack);
    void setPlaylistModel(QAbstractItemModel* playlistModel);

    void setMediaStatus(QMediaPlayer::MediaStatus mediaStatus);
    void setPlaybackState(QMediaPlayer::PlaybackState playbackState);
    void setTrackError(QMediaPlayer::Error trackError);

    void setDuration(qint64 duration);
    void setSeekable(bool seekable);
    void setPosition(qint64 position);

    void saveForUndoClearPlaylist();
    void restoreForUndoClearPlaylist();
    void setTrackControlPosition(int trackControlPosition);
    void setPersistentState(const QVariantMap& persistentState);

    void ensurePause();
    void ensurePlay();
    void requestPlay();
    void playPause();
    void stop();

    void trackSeek(int newPosition);
    void playlistFinished();
    void tracksDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

private:
    void notifyTrackSourceProperty();

    template <typename F>
    constexpr void enqueue(F&& op) {
        QTimer::singleShot(0, this, std::forward<F>(op));
    }

    void restorePreviousState();

    QPersistentModelIndex m_currentTrack;
    QPersistentModelIndex m_previousTrack;

    std::unique_ptr<TrackMetadata> m_trackMetadata;
    QAbstractItemModel* m_playlistModel = nullptr;

    QVariant m_previousTrackSource;
    QMediaPlayer::MediaStatus m_mediaStatus = QMediaPlayer::NoMedia;
    QMediaPlayer::PlaybackState m_playbackState = QMediaPlayer::StoppedState;
    QMediaPlayer::Error m_trackError = QMediaPlayer::NoError;

    bool m_isPlaying = false;
    bool m_skippingCurrentTrack = false;

    qint64 m_duration = 0;
    qint64 m_position = 0;
    bool m_seekable = false;

    QVariantMap m_persistentState;
    bool m_undoPlayingState = false;
    qint64 m_undoTrackPosition = 0;
};

#endif // ACTIVETRACKMANAGER_H
