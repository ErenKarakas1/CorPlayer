#ifndef PLAYLISTPROXYMODEL_HPP
#define PLAYLISTPROXYMODEL_HPP

#include "metadata.hpp"
#include "playerutils.hpp"

#include <QMediaPlayer>
#include <QQmlEngine>
#include <QSortFilterProxyModel>

class Library;
class PlaylistModel;
class PlaylistProxyModelPrivate;

class PlaylistProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Only available through CorPlayer")

public:
    enum RepeatMode { None, CurrentTrack, Playlist };
    Q_ENUM(RepeatMode);

    enum ShuffleMode { NoShuffle, Track };
    Q_ENUM(ShuffleMode);

private:
    Q_PROPERTY(QVariantMap persistentState READ persistentState WRITE setPersistentState NOTIFY persistentStateChanged)
    Q_PROPERTY(QPersistentModelIndex previousTrack READ previousTrack NOTIFY previousTrackChanged)
    Q_PROPERTY(QPersistentModelIndex currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QPersistentModelIndex nextTrack READ nextTrack NOTIFY nextTrackChanged)
    Q_PROPERTY(RepeatMode repeatMode READ repeatMode WRITE setRepeatMode NOTIFY repeatModeChanged)
    Q_PROPERTY(ShuffleMode shuffleMode READ shuffleMode WRITE setShuffleMode NOTIFY shuffleModeChanged)
    Q_PROPERTY(int totalTracksDuration READ totalTracksDuration NOTIFY totalTracksDurationChanged)
    Q_PROPERTY(int remainingTracksDuration READ remainingTracksDuration NOTIFY remainingTracksDurationChanged)
    Q_PROPERTY(int remainingTracks READ remainingTracks NOTIFY remainingTracksChanged)
    Q_PROPERTY(int tracksCount READ tracksCount NOTIFY tracksCountChanged)

public:
    explicit PlaylistProxyModel(Library* library, QObject* parent = nullptr);
    ~PlaylistProxyModel() override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    [[nodiscard]] QItemSelection mapSelectionFromSource(const QItemSelection& sourceSelection) const override;
    [[nodiscard]] QItemSelection mapSelectionToSource(const QItemSelection& proxySelection) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
    [[nodiscard]] int mapRowFromSource(int sourceRow) const;
    [[nodiscard]] int mapRowToSource(int proxyRow) const;
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] bool hasChildren(const QModelIndex& parent) const override;

    void setPlaylistModel(PlaylistModel* playlistModel);

    [[nodiscard]] QPersistentModelIndex previousTrack() const;
    [[nodiscard]] QPersistentModelIndex currentTrack() const;
    [[nodiscard]] QPersistentModelIndex nextTrack() const;
    [[nodiscard]] RepeatMode repeatMode() const;
    [[nodiscard]] ShuffleMode shuffleMode() const;
    [[nodiscard]] int totalTracksDuration() const;
    [[nodiscard]] int remainingTracksDuration() const;
    [[nodiscard]] int remainingTracks() const;
    [[nodiscard]] int tracksCount() const;
    [[nodiscard]] QVariantMap persistentState() const;

public Q_SLOTS:
    void enqueue(const QList<Metadata::TrackFields>& newEntries, PlayerUtils::PlaylistEnqueueMode enqueueMode,
                 PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

    void loadPlaylistFromDatabase(quint64 playlistId);
    void loadPlaylistFromFile(const QUrl& fileName);
    void clearPlaylist();
    void skipNextTrack(PlayerUtils::SkipReason reason = PlayerUtils::SkipReason::Automatic);
    void skipPreviousTrack(qint64 position);
    void switchTo(int row);
    void removeSelection(const QList<int>& selection);
    void moveRow(int from, int to);
    void setRepeatMode(PlaylistProxyModel::RepeatMode repeatMode);
    void setShuffleMode(PlaylistProxyModel::ShuffleMode shuffleMode);
    void setPersistentState(const QVariantMap& persistentState);
    void trackInError(const QUrl& sourceInError, QMediaPlayer::Error error);

Q_SIGNALS:
    void persistentStateChanged();
    void previousTrackChanged(const QPersistentModelIndex& previousTrack);
    void currentTrackChanged(const QPersistentModelIndex& currentTrack);
    void nextTrackChanged(const QPersistentModelIndex& nextTrack);
    void repeatModeChanged();
    void shuffleModeChanged();
    void totalTracksDurationChanged();
    void remainingTracksDurationChanged();
    void remainingTracksChanged();
    void tracksCountChanged();
    void currentTrackDataChanged();
    void ensurePlay();
    void playlistFinished();
    void playlistImported(quint64 playlistId);
    void seek(qint64 position);

private Q_SLOTS:
    void sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex& parent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex& parent, int start, int end, const QModelIndex& destParent,
                                  int destRow);
    void sourceRowsMoved(const QModelIndex& parent, int start, int end, const QModelIndex& destParent, int destRow);
    void sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles);
    void sourceLayoutAboutToBeChanged();
    void sourceLayoutChanged();
    void sourceModelAboutToBeReset();
    void sourceModelReset();

private:
    void setSourceModel(QAbstractItemModel* sourceModel) override;
    void determineTracks();
    void notifyCurrentTrackChanged();
    void determineAndNotifyPreviousAndNextTracks();
    [[nodiscard]] QVariantList getRandomMappingForRestore() const;
    void restoreShuffleMode(ShuffleMode mode, const QVariantList& mapping);

    std::unique_ptr<PlaylistProxyModelPrivate> pp;
};

#endif // PLAYLISTPROXYMODEL_HPP
