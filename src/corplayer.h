#ifndef CORPLAYER_H
#define CORPLAYER_H

#include "metadata.hpp"
#include "playerutils.hpp"

#include <QObject>
#include <QQmlEngine>
#include <QString>

#include <memory>

class QAction;
class DatabaseManager;
class TracksWatchdog;
class PlaylistModel;
class TrackPlaylistProxyModel;
class MediaPlayerWrapper;
class ActiveTrackManager;
class PlayerManager;
class QAbstractItemModel;
class CorPlayerPrivate;

class CorPlayer : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // clang-format off
    Q_PROPERTY(DatabaseManager *databaseManager READ databaseManager NOTIFY databaseManagerChanged)
    Q_PROPERTY(TracksWatchdog *tracksWatchdog READ tracksWatchdog NOTIFY tracksWatchdogChanged)
    Q_PROPERTY(PlaylistModel *playlistModel READ playlistModel NOTIFY playlistModelChanged)

    Q_PROPERTY(TrackPlaylistProxyModel *trackPlaylistProxyModel
        READ trackPlaylistProxyModel NOTIFY trackPlaylistProxyModelChanged)

    Q_PROPERTY(MediaPlayerWrapper *mediaPlayer READ mediaPlayer NOTIFY mediaPlayerChanged)
    Q_PROPERTY(ActiveTrackManager *trackManager READ trackManager NOTIFY trackManagerChanged)
    Q_PROPERTY(PlayerManager *playerManager READ playerManager NOTIFY playerManagerChanged)
    // clang-format on

public:
    explicit CorPlayer(QObject* parent = nullptr);
    ~CorPlayer() override;

    [[nodiscard]] DatabaseManager* databaseManager() const;
    [[nodiscard]] TracksWatchdog* tracksWatchdog() const;
    [[nodiscard]] PlaylistModel* playlistModel() const;
    [[nodiscard]] TrackPlaylistProxyModel* trackPlaylistProxyModel() const;
    [[nodiscard]] MediaPlayerWrapper* mediaPlayer() const;
    [[nodiscard]] ActiveTrackManager *trackManager() const;
    [[nodiscard]] PlayerManager *playerManager() const;

Q_SIGNALS:
    void databaseManagerChanged();
    void tracksWatchdogChanged();
    void playlistModelChanged();
    void trackPlaylistProxyModelChanged();
    void mediaPlayerChanged();
    void trackManagerChanged();
    void playerManagerChanged();

    void enqueue(const Metadata::EntryFieldsList& newEntries, PlayerUtils::PlaylistEnqueueMode enqueueMode,
                 PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay);

    void initializationDone();

public Q_SLOTS:
    bool openFiles(const QList<QUrl>& files);
    bool openFiles(const QList<QUrl>& files, const QString& workingDirectory);
    void initialize();

private:
    void initializeModels();
    void initializePlayer();

    [[nodiscard]] Metadata::EntryFieldsList sanitizePlaylist(const Metadata::EntryFieldsList& entries,
                                                             const QString& workingDirectory) const;

    std::unique_ptr<CorPlayerPrivate> capp;
};

#endif // CORPLAYER_H
