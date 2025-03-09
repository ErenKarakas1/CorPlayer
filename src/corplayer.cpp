#include "corplayer.h"

#include "models/playlistmodel.hpp"
#include "models/playlistproxymodel.hpp"

#include "models/playlistcollectionmodel.hpp"
#include "models/trackcollectionmodel.hpp"

#include "activetrackmanager.h"
#include "database/databasemanager.h"
#include "library/library.hpp"
#include "mediaplayerwrapper.h"
#include "playermanager.h"

#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMimeDatabase>
#include <QMimeType>
#include <QQmlComponent>
#include <QStandardPaths>
#include <QThread>
#include <QUrl>

class CorPlayerPrivate {
public:
    QThread m_databaseThread;
    DatabaseManager* m_dbManager = nullptr;
    std::unique_ptr<Library> m_library;

    std::unique_ptr<TrackCollectionModel> m_trackCollectionModel;
    std::unique_ptr<PlaylistCollectionModel> m_playlistCollectionModel;
    std::unique_ptr<PlaylistModel> m_playlistModel;
    std::unique_ptr<PlaylistProxyModel> m_playlistProxyModel;

    std::unique_ptr<MediaPlayerWrapper> m_mediaPlayer;
    std::unique_ptr<ActiveTrackManager> m_trackManager;
    std::unique_ptr<PlayerManager> m_playerManager;
};

CorPlayer::CorPlayer(QObject* parent) : QObject(parent), capp(std::make_unique<CorPlayerPrivate>()) {
    capp->m_databaseThread.start();
}

CorPlayer::~CorPlayer() {
    capp->m_databaseThread.quit();
    capp->m_databaseThread.wait();
}

bool CorPlayer::openFiles(const QList<QUrl>& files) {
    return openFiles(files, QDir::currentPath());
}

bool CorPlayer::openFiles(const QList<QUrl>& files, const QString& workingDirectory) {
    static const QMimeDatabase mimeDB;
    QList<Metadata::TrackFields> entries;

    for (const QUrl& file : files) {
        const QMimeType mime = mimeDB.mimeTypeForUrl(file);
        if (mime.name().startsWith(QStringLiteral("audio/"))) {
            auto entry = Metadata::TrackFields();
            entry.insert(Metadata::Fields::ElementType, PlayerUtils::FileName);
            entry.insert(Metadata::Fields::ResourceUrl, file);
            entries.emplace_back(std::move(entry));
        }
    }

    const auto targetEntries = sanitizePlaylist(entries, workingDirectory);
    if (targetEntries.isEmpty()) return false;

    capp->m_playlistProxyModel->enqueue(targetEntries, PlayerUtils::PlaylistEnqueueMode::AppendPlaylist,
                                        PlayerUtils::PlaylistEnqueueTriggerPlay::TriggerPlay);

    return true;
}

void CorPlayer::initialize() {
    initializeModels();
    initializePlayer();

    Q_EMIT initializationDone();
}

void CorPlayer::playTrack(const quint64 trackId) const {
    const auto track = capp->m_library->trackDatabase().fetchTrackFromId(trackId);
    if (!track.isValid()) return;

    capp->m_playlistProxyModel->enqueue({track}, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist,
                                        PlayerUtils::TriggerPlay);
}

void CorPlayer::initializeModels() {
    capp->m_dbManager = &DatabaseManager::instance();

    const auto dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const bool success = QDir().mkpath(dbPath);

    if (!success) {
        qWarning() << "Failed to create directory " << dbPath;
    }

    capp->m_dbManager->initialize(dbPath + QStringLiteral("/corplayer.db"));

    capp->m_library = std::make_unique<Library>();
    capp->m_library->initialize(capp->m_dbManager->dbConnectionPool());

    capp->m_trackCollectionModel = std::make_unique<TrackCollectionModel>(capp->m_library.get());
    Q_EMIT trackCollectionModelChanged();

    capp->m_playlistCollectionModel = std::make_unique<PlaylistCollectionModel>(capp->m_library.get());
    Q_EMIT playlistCollectionModelChanged();

    capp->m_playlistModel = std::make_unique<PlaylistModel>(capp->m_library.get());
    Q_EMIT playlistModelChanged();

    capp->m_playlistProxyModel = std::make_unique<PlaylistProxyModel>(capp->m_library.get());
    capp->m_playlistProxyModel->setPlaylistModel(capp->m_playlistModel.get());
    Q_EMIT playlistProxyModelChanged();

    // connect(capp->m_playlistModel.get(), &PlaylistModel::addNewUrl, capp->m_library.get(),
    // &Library::addTrackFromUrl);
}

void CorPlayer::initializePlayer() {
    capp->m_mediaPlayer = std::make_unique<MediaPlayerWrapper>();
    Q_EMIT mediaPlayerChanged();

    capp->m_trackManager = std::make_unique<ActiveTrackManager>();
    Q_EMIT trackManagerChanged();

    capp->m_playerManager = std::make_unique<PlayerManager>();
    Q_EMIT playerManagerChanged();

    capp->m_trackManager->setPlaylistModel(capp->m_playlistProxyModel.get());

    // clang-format off
    connect(capp->m_trackManager.get(), &ActiveTrackManager::playTrack, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::play);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::pauseTrack, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::pause);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::stopTrack, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::stop);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::seek, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seek);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::saveUndoPositionInWrapper, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::saveUndoPosition);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::restoreUndoPositionInWrapper, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::restoreUndoPosition);

    connect(capp->m_trackManager.get(), &ActiveTrackManager::skipNextTrack, capp->m_playlistProxyModel.get(), &PlaylistProxyModel::skipNextTrack);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::sourceInError, capp->m_playlistProxyModel.get(), &PlaylistProxyModel::trackInError);

    connect(capp->m_trackManager.get(), &ActiveTrackManager::trackSourceChanged, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::setSource);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::updateData, capp->m_playlistModel.get(), &PlaylistModel::setData);

    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::ensurePlay, capp->m_trackManager.get(), &ActiveTrackManager::ensurePlay);
    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::playlistFinished, capp->m_trackManager.get(), &ActiveTrackManager::playlistFinished);
    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::currentTrackChanged, capp->m_trackManager.get(), &ActiveTrackManager::setCurrentTrack);
    // connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::clearPlaylistPlayer, capp->m_trackManager.get(), &ActiveTrackManager::saveForUndoClearPlaylist);
    // connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::undoClearPlaylistPlayer, capp->m_trackManager.get(), &ActiveTrackManager::restoreForUndoClearPlaylist);
    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::seek, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seek);

    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::playbackStateChanged, capp->m_trackManager.get(), &ActiveTrackManager::setPlaybackState);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::statusChanged, capp->m_trackManager.get(), &ActiveTrackManager::setMediaStatus);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::errorChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackError);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::durationChanged, capp->m_trackManager.get(), &ActiveTrackManager::setDuration);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::positionChanged, capp->m_trackManager.get(), &ActiveTrackManager::setPosition);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seekableChanged, capp->m_trackManager.get(), &ActiveTrackManager::setSeekable);

    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::currentTrackChanged, capp->m_playerManager.get(), &PlayerManager::setCurrentTrack);
    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::previousTrackChanged, capp->m_playerManager.get(), &PlayerManager::setPreviousTrack);
    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::nextTrackChanged, capp->m_playerManager.get(), &PlayerManager::setNextTrack);

    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::playing, capp->m_playerManager.get(), &PlayerManager::playing);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::paused, capp->m_playerManager.get(), &PlayerManager::pausedOrStopped);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::stopped, capp->m_playerManager.get(), &PlayerManager::pausedOrStopped);

    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::currentTrackChanged, capp->m_trackManager->trackMetadata(), &TrackMetadata::setCurrentTrack);
    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::currentTrackDataChanged, capp->m_trackManager->trackMetadata(), &TrackMetadata::updateMetadata);

    connect(capp->m_playlistProxyModel.get(), &PlaylistProxyModel::playlistImported, capp->m_playlistCollectionModel.get(), &PlaylistCollectionModel::onPlaylistModified);
    // clang-format on
}

QList<Metadata::TrackFields> CorPlayer::sanitizePlaylist(QList<Metadata::TrackFields>& entries,
                                                         const QString& workingDirectory) {
    QList<Metadata::TrackFields> result;
    for (auto& entry : entries) {
        const QUrl url = entry.get(Metadata::Fields::ResourceUrl).toUrl();
        if (url.scheme().isEmpty() || url.isLocalFile()) {
            auto newFile = QFileInfo(url.toLocalFile());

            if (url.scheme().isEmpty()) {
                newFile = QFileInfo(url.toString());
            }

            if (newFile.isRelative()) {
                if (url.scheme().isEmpty()) {
                    newFile.setFile(workingDirectory, url.toString());
                } else {
                    newFile.setFile(workingDirectory, url.toLocalFile());
                }
            }

            if (newFile.exists()) {
                entry.insert(Metadata::Fields::ResourceUrl, QUrl::fromLocalFile(newFile.absoluteFilePath()));
                result.emplace_back(std::move(entry));
            }
        } else {
            result.emplace_back(std::move(entry));
        }
    }
    return result;
}

TrackCollectionModel* CorPlayer::trackCollectionModel() const {
    return capp->m_trackCollectionModel.get();
}

PlaylistCollectionModel* CorPlayer::playlistCollectionModel() const {
    return capp->m_playlistCollectionModel.get();
}

PlaylistModel* CorPlayer::playlistModel() const {
    return capp->m_playlistModel.get();
}

PlaylistProxyModel* CorPlayer::playlistProxyModel() const {
    return capp->m_playlistProxyModel.get();
}

MediaPlayerWrapper* CorPlayer::mediaPlayer() const {
    return capp->m_mediaPlayer.get();
}

ActiveTrackManager* CorPlayer::trackManager() const {
    return capp->m_trackManager.get();
}

PlayerManager* CorPlayer::playerManager() const {
    return capp->m_playerManager.get();
}

#include "moc_corplayer.cpp"
