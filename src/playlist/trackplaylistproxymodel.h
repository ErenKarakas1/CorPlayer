#ifndef TRACKPLAYLISTPROXYMODEL_H
#define TRACKPLAYLISTPROXYMODEL_H

#include "metadatafields.h"
#include "playerutils.h"

#include <QAbstractProxyModel>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMimeType>
#include <QQmlEngine>

#include <memory>

class TrackPlaylist;
class TrackPlaylistProxyModelPrivate;

class TrackPlaylistProxyModel : public QAbstractProxyModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Repeat {
        None,
        CurrentTrack,
        Playlist
    };

    Q_ENUM(Repeat);

    enum Shuffle {
        NoShuffle,
        Track
    };

    Q_ENUM(Shuffle);

private:
    Q_PROPERTY(QVariantMap persistentState READ persistentState WRITE setPersistentState NOTIFY persistentStateChanged)
    Q_PROPERTY(QPersistentModelIndex previousTrack READ previousTrack NOTIFY previousTrackChanged)
    Q_PROPERTY(QPersistentModelIndex currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QPersistentModelIndex nextTrack READ nextTrack NOTIFY nextTrackChanged)
    Q_PROPERTY(Repeat repeatMode READ repeatMode WRITE setRepeatMode NOTIFY repeatModeChanged)
    Q_PROPERTY(Shuffle shuffleMode READ shuffleMode WRITE setShuffleMode NOTIFY shuffleModeChanged)

    // in milliseconds
    Q_PROPERTY(int totalTracksDuration READ totalTracksDuration NOTIFY totalTracksDurationChanged)

    // in milliseconds
    Q_PROPERTY(int remainingTracksDuration READ remainingTracksDuration NOTIFY remainingTracksDurationChanged)
    Q_PROPERTY(int remainingTracks READ remainingTracks NOTIFY remainingTracksChanged)
    Q_PROPERTY(int currentTrackRow READ currentTrackRow NOTIFY currentTrackRowChanged)
    Q_PROPERTY(int tracksCount READ tracksCount NOTIFY tracksCountChanged)
    Q_PROPERTY(bool partiallyLoaded READ partiallyLoaded NOTIFY partiallyLoadedChanged RESET resetPartiallyLoaded)
    Q_PROPERTY(bool canOpenLoadedPlaylist READ canOpenLoadedPlaylist NOTIFY canOpenLoadedPlaylistChanged)

public:
    explicit TrackPlaylistProxyModel(QObject *parent = nullptr);
    ~TrackPlaylistProxyModel() override;
    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    [[nodiscard]] QItemSelection mapSelectionFromSource(const QItemSelection &sourceSelection) const override;
    [[nodiscard]] QItemSelection mapSelectionToSource(const QItemSelection &proxySelection) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    [[nodiscard]] int mapRowFromSource(int sourceRow) const;
    [[nodiscard]] int mapRowToSource(int proxyRow) const;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &child) const override;
    [[nodiscard]] bool hasChildren(const QModelIndex &parent) const override;
    void setPlaylistModel(TrackPlaylist *playlistModel);
    [[nodiscard]] QPersistentModelIndex previousTrack() const;
    [[nodiscard]] QPersistentModelIndex currentTrack() const;
    [[nodiscard]] QPersistentModelIndex nextTrack() const;
    [[nodiscard]] Repeat repeatMode() const;
    [[nodiscard]] Shuffle shuffleMode() const;
    [[nodiscard]] int totalTracksDuration() const;
    [[nodiscard]] int remainingTracksDuration() const;
    [[nodiscard]] int remainingTracks() const;
    [[nodiscard]] int currentTrackRow() const;
    [[nodiscard]] int tracksCount() const;
    [[nodiscard]] QVariantMap persistentState() const;
    [[nodiscard]] bool partiallyLoaded() const;
    [[nodiscard]] bool canOpenLoadedPlaylist() const;

public Q_SLOTS:
    void enqueue(const QUrl &entryUrl, PlayerUtils::PlaylistEnqueueMode enqueueMode,
                 PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

    void enqueue(const MetadataFields::MusicMetadataField &newEntry, const QString &newEntryTitle,
                 PlayerUtils::PlaylistEnqueueMode enqueueMode, PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

    void enqueue(const MetadataFields::EntryMetadataList &newEntries, PlayerUtils::PlaylistEnqueueMode enqueueMode,
                 PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

    void setRepeatMode(TrackPlaylistProxyModel::Repeat newMode);
    void setShuffleMode(TrackPlaylistProxyModel::Shuffle value);
    void trackInError(const QUrl &sourceInError, QMediaPlayer::Error playerError) const;
    void skipNextTrack(PlayerUtils::SkipReason reason = PlayerUtils::SkipReason::Automatic);
    void skipPreviousTrack(qint64 position);
    void switchTo(int row);
    void removeSelection(QList<int> selection) const;
    void removeRow(int row) const;
    void moveRow(int from, int to);
    void clearPlaylist();
    void undoClearPlaylist();
    [[nodiscard]] bool savePlaylist(const QUrl &fileName) const;
    void loadPlaylist(const QUrl &fileName);

    void loadPlaylist(const QUrl &fileName, PlayerUtils::PlaylistEnqueueMode enqueueMode,
                      PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

    void setPersistentState(const QVariantMap &persistentState);
    void resetPartiallyLoaded();
    [[nodiscard]] int indexForTrackUrl(const QUrl &url) const;
    void switchToTrackUrl(const QUrl &url, PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

Q_SIGNALS:
    void previousTrackChanged(const QPersistentModelIndex &previousTrack);
    void currentTrackChanged(const QPersistentModelIndex &currentTrack);
    void nextTrackChanged(const QPersistentModelIndex &nextTrack);
    void previousTrackDataChanged();
    void currentTrackDataChanged();
    void nextTrackDataChanged();
    void repeatModeChanged();
    void shuffleModeChanged();
    void totalTracksDurationChanged();
    void remainingTracksDurationChanged();
    void remainingTracksChanged();
    void ensurePlay();
    void requestPlay();
    void currentTrackRowChanged();
    void tracksCountChanged();
    void playlistFinished();
    void playlistLoaded();
    void playlistLoadFailed();
    void persistentStateChanged();
    void clearPlaylistPlayer();
    void undoClearPlaylistPlayer();
    void displayUndoNotification();
    void hideUndoNotification();
    void seek(qint64 position);
    void partiallyLoadedChanged();
    void canOpenLoadedPlaylistChanged();

private Q_SLOTS:
    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);

    void sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent,
                                  int destRow);

    void sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int destRow);

    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);
    void sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    void sourceLayoutAboutToBeChanged();
    void sourceLayoutChanged();
    void sourceModelAboutToBeReset();
    void sourceModelReset();

private:
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    void determineTracks();
    void notifyCurrentTrackRowChanged();
    void notifyCurrentTrackChanged();
    void determineAndNotifyPreviousAndNextTracks();
    [[nodiscard]] QVariantList getRandomMappingForRestore() const;
    void restoreShuffleMode(Shuffle mode, QVariantList mapping);

    void loadLocalFile(MetadataFields::EntryMetadataList &newTracks, QSet<QString> &processedFiles,
                       const QFileInfo &fileInfo);

    void loadLocalPlaylist(MetadataFields::EntryMetadataList &newTracks, QSet<QString> &processedFiles,
                           const QUrl &fileName, const QByteArray &fileContent);

    void loadLocalDirectory(MetadataFields::EntryMetadataList &newTracks, QSet<QString> &processedFiles,
                            const QUrl &dirName);

    int filterLocalPlaylist(QList<QUrl> &result, const QUrl &playlistUrl);

    int m_seekToBeginningDelay = 2000;
    std::unique_ptr<TrackPlaylistProxyModelPrivate> tpp;
};

#endif // TRACKPLAYLISTPROXYMODEL_H
