#ifndef CORPLAYER_H
#define CORPLAYER_H

#include "metadatafields.h"
#include "playerutils.h"

#include <QObject>
#include <QQmlEngine>
#include <QString>

#include <memory>

class QAction;
class TrackPlaylist;
class TrackPlaylistProxyModel;
class MediaPlayerWrapper;
class ActiveTrackManager;
class PlayerManager;
class TrackMetadataManager;
class TracksWatchdog;
class QAbstractItemModel;
class CorPlayerPrivate;

class CorPlayer : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // clang-format off
    Q_PROPERTY(TrackPlaylist *trackPlaylist READ trackPlaylist NOTIFY trackPlaylistChanged)

    Q_PROPERTY(TrackPlaylistProxyModel *trackPlaylistProxyModel
        READ trackPlaylistProxyModel NOTIFY trackPlaylistProxyModelChanged)

    Q_PROPERTY(MediaPlayerWrapper *mediaPlayer READ mediaPlayer NOTIFY mediaPlayerChanged)
    Q_PROPERTY(ActiveTrackManager *trackManager READ trackManager NOTIFY trackManagerChanged)
    Q_PROPERTY(PlayerManager *playerManager READ playerManager NOTIFY playerManagerChanged)
    Q_PROPERTY(TrackMetadataManager *metadataManager READ metadataManager NOTIFY metadataManagerChanged)
    // clang-format on

public:
    explicit CorPlayer(QObject *parent = nullptr);
    ~CorPlayer() override;

    [[nodiscard]] TrackPlaylist *trackPlaylist() const;
    [[nodiscard]] TrackPlaylistProxyModel *trackPlaylistProxyModel() const;
    [[nodiscard]] MediaPlayerWrapper *mediaPlayer() const;
    [[nodiscard]] ActiveTrackManager *trackManager() const;
    [[nodiscard]] PlayerManager *playerManager() const;
    [[nodiscard]] TrackMetadataManager *metadataManager() const;

Q_SIGNALS:
    void trackPlaylistChanged();
    void trackPlaylistProxyModelChanged();
    void mediaPlayerChanged();
    void trackManagerChanged();
    void playerManagerChanged();
    void metadataManagerChanged();

    void enqueue(const MetadataFields::EntryMetadataList &tracks, PlayerUtils::PlaylistEnqueueMode enqueueMode,
                 PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

    void initializationDone();

public Q_SLOTS:
    bool openFiles(const QList<QUrl> &files);
    bool openFiles(const QList<QUrl> &files, const QString &workingDirectory);
    void initialize();

private:
    void initializeModels();
    void initializePlayer();

    [[nodiscard]] MetadataFields::EntryMetadataList sanitizePlaylist(const MetadataFields::EntryMetadataList &tracks,
                                                                     const QString &workingDirectory) const;

    std::unique_ptr<CorPlayerPrivate> capp;
};

#endif // CORPLAYER_H
