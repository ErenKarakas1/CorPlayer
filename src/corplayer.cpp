#include "corplayer.h"

#include "playlist/playlistmodel.hpp"
#include "playlist/trackplaylistproxymodel.h"

#include "database/databasemanager.h"
#include "trackswatchdog.h"
#include "mediaplayerwrapper.h"
#include "activetrackmanager.h"
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
    QThread m_indexerThread;

    DatabaseManager* m_dbManager = nullptr;
    std::unique_ptr<TracksWatchdog> m_tracksWatchdog;
    std::unique_ptr<PlaylistModel> m_playlistModel;
    std::unique_ptr<TrackPlaylistProxyModel> m_trackPlaylistProxyModel;
    std::unique_ptr<MediaPlayerWrapper> m_mediaPlayer;
    std::unique_ptr<ActiveTrackManager> m_trackManager;
    std::unique_ptr<PlayerManager> m_playerManager;
};

CorPlayer::CorPlayer(QObject* parent) : QObject(parent), capp(std::make_unique<CorPlayerPrivate>()) {
    capp->m_databaseThread.start();
    capp->m_indexerThread.start();
}

CorPlayer::~CorPlayer() {
    capp->m_indexerThread.quit();
    capp->m_indexerThread.wait();

    capp->m_databaseThread.quit();
    capp->m_databaseThread.wait();
}

bool CorPlayer::openFiles(const QList<QUrl>& files) {
    return openFiles(files, QDir::currentPath());
}

bool CorPlayer::openFiles(const QList<QUrl>& files, const QString& workingDirectory) {
    auto tracks = Metadata::EntryFieldsList{};
    const QMimeDatabase mimeDB;

    for (const auto& file : files) {
        const QMimeType mime = mimeDB.mimeTypeForUrl(file);
        if (PlayerUtils::isPlaylist(mime)) {
            capp->m_trackPlaylistProxyModel->loadPlaylist(file);
        } else if (mime.name().startsWith(QStringLiteral("audio/"))) {
            auto entry = Metadata::TrackFields();
            entry.insert(Metadata::Fields::ElementType, PlayerUtils::FileName);
            entry.insert(Metadata::Fields::ResourceUrl, file);
            tracks.emplace_back(Metadata::EntryFields{entry, {}, {}});
        }
    }

    const auto targetFiles = sanitizePlaylist(tracks, workingDirectory);

    if (!targetFiles.isEmpty()) {
        Q_EMIT enqueue(targetFiles, PlayerUtils::PlaylistEnqueueMode::AppendPlaylist,
                       PlayerUtils::PlaylistEnqueueTriggerPlay::TriggerPlay);
    }

    return tracks.count() == targetFiles.count();
}

void CorPlayer::initialize() {
    initializeModels();
    initializePlayer();

    Q_EMIT initializationDone();
}

void CorPlayer::initializeModels() {
    capp->m_playlistModel = std::make_unique<PlaylistModel>();
    Q_EMIT playlistModelChanged();

    capp->m_trackPlaylistProxyModel = std::make_unique<TrackPlaylistProxyModel>();
    capp->m_trackPlaylistProxyModel->setPlaylistModel(capp->m_playlistModel.get());
    Q_EMIT trackPlaylistProxyModelChanged();

    capp->m_dbManager = &DatabaseManager::instance();

    const auto dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const bool success = QDir().mkpath(dbPath);

    if (!success) {
        qWarning() << "Failed to create directory " << dbPath;
    }

    capp->m_dbManager->initialize(dbPath + QStringLiteral("/corplayer.db"));

    Q_EMIT databaseManagerChanged();

    if (capp->m_tracksWatchdog) return;

    capp->m_tracksWatchdog = std::make_unique<TracksWatchdog>();
    capp->m_tracksWatchdog->moveToThread(&capp->m_databaseThread);
    QMetaObject::invokeMethod(capp->m_tracksWatchdog.get(), "initDatabase", Qt::QueuedConnection,
                              Q_ARG(std::shared_ptr<DbConnectionPool>, capp->m_dbManager->dbConnectionPool()));

    Q_EMIT tracksWatchdogChanged();

    connect(capp->m_tracksWatchdog.get(), &TracksWatchdog::trackHasChanged, capp->m_playlistModel.get(),
            &PlaylistModel::trackChanged);
    connect(capp->m_playlistModel.get(), &PlaylistModel::addNewUrl, capp->m_tracksWatchdog.get(),
            &TracksWatchdog::addNewUrl);

    connect(this, &CorPlayer::enqueue, capp->m_trackPlaylistProxyModel.get(),
            static_cast<void (TrackPlaylistProxyModel::*)(
                const Metadata::EntryFieldsList&, PlayerUtils::PlaylistEnqueueMode,
                PlayerUtils::PlaylistEnqueueTriggerPlay)>(&TrackPlaylistProxyModel::enqueue));
}

void CorPlayer::initializePlayer() {
    capp->m_mediaPlayer = std::make_unique<MediaPlayerWrapper>();
    Q_EMIT mediaPlayerChanged();

    capp->m_trackManager = std::make_unique<ActiveTrackManager>();
    Q_EMIT trackManagerChanged();

    capp->m_playerManager = std::make_unique<PlayerManager>();
    Q_EMIT playerManagerChanged();

    capp->m_trackManager->setPlaylistModel(capp->m_trackPlaylistProxyModel.get());

    // clang-format off
    connect(capp->m_trackManager.get(), &ActiveTrackManager::playTrack, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::play);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::pauseTrack, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::pause);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::stopTrack, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::stop);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::seek, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seek);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::saveUndoPositionInWrapper, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::saveUndoPosition);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::restoreUndoPositionInWrapper, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::restoreUndoPosition);

    connect(capp->m_trackManager.get(), &ActiveTrackManager::skipNextTrack, capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::skipNextTrack);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::sourceInError, capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::trackInError);

    connect(capp->m_trackManager.get(), &ActiveTrackManager::trackSourceChanged, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::setSource);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::updateData, capp->m_playlistModel.get(), &PlaylistModel::setData);

    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::ensurePlay, capp->m_trackManager.get(), &ActiveTrackManager::ensurePlay);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::requestPlay, capp->m_trackManager.get(), &ActiveTrackManager::requestPlay);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::playlistFinished, capp->m_trackManager.get(), &ActiveTrackManager::playlistFinished);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackChanged, capp->m_trackManager.get(), &ActiveTrackManager::setCurrentTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::clearPlaylistPlayer, capp->m_trackManager.get(), &ActiveTrackManager::saveForUndoClearPlaylist);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::undoClearPlaylistPlayer, capp->m_trackManager.get(), &ActiveTrackManager::restoreForUndoClearPlaylist);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::seek, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seek);

    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::playbackStateChanged, capp->m_trackManager.get(), &ActiveTrackManager::setPlaybackState);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::statusChanged, capp->m_trackManager.get(), &ActiveTrackManager::setMediaStatus);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::errorChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackError);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::durationChanged, capp->m_trackManager.get(), &ActiveTrackManager::setDuration);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::positionChanged, capp->m_trackManager.get(), &ActiveTrackManager::setPosition);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seekableChanged, capp->m_trackManager.get(), &ActiveTrackManager::setSeekable);

    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackChanged, capp->m_playerManager.get(), &PlayerManager::setCurrentTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::previousTrackChanged, capp->m_playerManager.get(), &PlayerManager::setPreviousTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::nextTrackChanged, capp->m_playerManager.get(), &PlayerManager::setNextTrack);

    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::playing, capp->m_playerManager.get(), &PlayerManager::playing);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::paused, capp->m_playerManager.get(), &PlayerManager::pausedOrStopped);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::stopped, capp->m_playerManager.get(), &PlayerManager::pausedOrStopped);

    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackChanged, capp->m_trackManager->trackMetadata(), &TrackMetadata::setCurrentTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackDataChanged, capp->m_trackManager->trackMetadata(), &TrackMetadata::updateMetadata);
    // clang-format on
}

Metadata::EntryFieldsList CorPlayer::sanitizePlaylist(const Metadata::EntryFieldsList& entries,
                                                      const QString& workingDirectory) const {
    auto result = Metadata::EntryFieldsList{};
    for (const auto& entry : entries) {
        auto url = entry.trackFields.get(Metadata::Fields::ResourceUrl).toUrl();
        auto entryUrl = entry.url.isValid() ? entry.url : url;
        if (entryUrl.scheme().isEmpty() || entryUrl.isLocalFile()) {
            auto newFile = QFileInfo(entryUrl.toLocalFile());

            if (entryUrl.scheme().isEmpty()) {
                newFile = QFileInfo(entryUrl.toString());
            }

            if (newFile.isRelative()) {
                if (entryUrl.scheme().isEmpty()) {
                    newFile.setFile(workingDirectory, entryUrl.toString());
                } else {
                    newFile.setFile(workingDirectory, entryUrl.toLocalFile());
                }
            }

            if (newFile.exists()) {
                auto trackMetadata = entry.trackFields;
                trackMetadata.insert(Metadata::Fields::ResourceUrl, QUrl::fromLocalFile(newFile.absoluteFilePath()));
                result.push_back({trackMetadata, entry.title, {}});
            }
        } else {
            result.push_back(entry);
        }
    }
    return result;
}

DatabaseManager* CorPlayer::databaseManager() const {
    return capp->m_dbManager;
}

TracksWatchdog* CorPlayer::tracksWatchdog() const {
    return capp->m_tracksWatchdog.get();
}

PlaylistModel* CorPlayer::playlistModel() const {
    return capp->m_playlistModel.get();
}

TrackPlaylistProxyModel* CorPlayer::trackPlaylistProxyModel() const {
    return capp->m_trackPlaylistProxyModel.get();
}

MediaPlayerWrapper* CorPlayer::mediaPlayer() const {
    return capp->m_mediaPlayer.get();
}

ActiveTrackManager *CorPlayer::trackManager() const {
    return capp->m_trackManager.get();
}

PlayerManager *CorPlayer::playerManager() const {
    return capp->m_playerManager.get();
}

#include "moc_corplayer.cpp"
