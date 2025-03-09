#ifndef CORPLAYER_H
#define CORPLAYER_H

#include "metadata.hpp"
#include "playerutils.hpp"

#include <QObject>
#include <QQmlEngine>
#include <QString>

#include <memory>

class DatabaseManager;
class MediaPlayerWrapper;
class ActiveTrackManager;
class PlayerManager;
class TrackCollectionModel;
class PlaylistCollectionModel;
class PlaylistModel;
class PlaylistProxyModel;
class CorPlayerPrivate;

class CorPlayer : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // clang-format off
    Q_PROPERTY(TrackCollectionModel* trackCollectionModel READ trackCollectionModel NOTIFY trackCollectionModelChanged)
    Q_PROPERTY(PlaylistCollectionModel* playlistCollectionModel READ playlistCollectionModel NOTIFY playlistCollectionModelChanged)

    Q_PROPERTY(PlaylistModel* playlistModel READ playlistModel NOTIFY playlistModelChanged)
    Q_PROPERTY(PlaylistProxyModel* playlistProxyModel READ playlistProxyModel NOTIFY playlistProxyModelChanged)

    Q_PROPERTY(MediaPlayerWrapper* mediaPlayer READ mediaPlayer NOTIFY mediaPlayerChanged)
    Q_PROPERTY(ActiveTrackManager* trackManager READ trackManager NOTIFY trackManagerChanged)
    Q_PROPERTY(PlayerManager* playerManager READ playerManager NOTIFY playerManagerChanged)
    // clang-format on

public:
    explicit CorPlayer(QObject* parent = nullptr);
    ~CorPlayer() override;

    [[nodiscard]] TrackCollectionModel* trackCollectionModel() const;
    [[nodiscard]] PlaylistCollectionModel* playlistCollectionModel() const;
    [[nodiscard]] PlaylistModel* playlistModel() const;
    [[nodiscard]] PlaylistProxyModel* playlistProxyModel() const;
    [[nodiscard]] MediaPlayerWrapper* mediaPlayer() const;
    [[nodiscard]] ActiveTrackManager* trackManager() const;
    [[nodiscard]] PlayerManager* playerManager() const;

Q_SIGNALS:
    void trackCollectionModelChanged();
    void playlistCollectionModelChanged();
    void playlistModelChanged();
    void playlistProxyModelChanged();
    void mediaPlayerChanged();
    void trackManagerChanged();
    void playerManagerChanged();
    void initializationDone();

public Q_SLOTS:
    bool openFiles(const QList<QUrl>& files);
    bool openFiles(const QList<QUrl>& files, const QString& workingDirectory);
    void initialize();
    void playTrack(quint64 trackId) const;

private:
    void initializeModels();
    void initializePlayer();

    [[nodiscard]] static QList<Metadata::TrackFields> sanitizePlaylist(QList<Metadata::TrackFields>& entries,
                                                                       const QString& workingDirectory);

    std::unique_ptr<CorPlayerPrivate> capp;
};

#endif // CORPLAYER_H
