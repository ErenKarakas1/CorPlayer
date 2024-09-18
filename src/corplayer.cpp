#include "corplayer.h"

#include "playlist/trackplaylist.h"
#include "playlist/trackplaylistproxymodel.h"

#include "activetrackmanager.h"
#include "mediaplayerwrapper.h"
#include "playermanager.h"
#include "trackmetadatamanager.h"
#include "trackswatchdog.h"

#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPointer>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QUrl>

class CorPlayerPrivate {
public:
    explicit CorPlayerPrivate(QObject *parent) {
        Q_UNUSED(parent);
    }

    std::unique_ptr<TrackPlaylist> m_trackPlaylist;
    std::unique_ptr<TrackPlaylistProxyModel> m_trackPlaylistProxyModel;
    std::unique_ptr<MediaPlayerWrapper> m_mediaPlayer;
    std::unique_ptr<ActiveTrackManager> m_trackManager;
    std::unique_ptr<PlayerManager> m_playerManager;
    std::unique_ptr<TrackMetadataManager> m_metadataManager;
    std::unique_ptr<TracksWatchdog> m_listener;
};

CorPlayer::CorPlayer(QObject *parent) : QObject(parent), capp(std::make_unique<CorPlayerPrivate>(this)) {}

CorPlayer::~CorPlayer() = default;

bool CorPlayer::openFiles(const QList<QUrl> &files) {
    return openFiles(files, QDir::currentPath());
}

bool CorPlayer::openFiles(const QList<QUrl> &files, const QString &workingDirectory) {
    auto tracks = MetadataFields::EntryMetadataList{};
    const QMimeDatabase mimeDB;

    for (const auto &file : files) {
        const QMimeType mime = mimeDB.mimeTypeForUrl(file);
        if (PlayerUtils::isPlaylist(mime)) {
            qDebug() << "openFiles: loading playlist: " << file;
            capp->m_trackPlaylistProxyModel->loadPlaylist(file);
        } else if (mime.name().startsWith(QStringLiteral("audio/"))) {
            tracks.push_back(MetadataFields::EntryMetadata{{{MetadataFields::ElementTypeRole, PlayerUtils::FileName},
                                                            {MetadataFields::ResourceRole, file},
                                                            {},
                                                            {}}});
        }
    }

    qDebug() << "openFiles: " << tracks.count() << " tracks before sanitize";
    const auto targetFiles = sanitizePlaylist(tracks, workingDirectory);

    if (!targetFiles.isEmpty()) {
        Q_EMIT enqueue(targetFiles, PlayerUtils::PlaylistEnqueueMode::AppendPlaylist,
                       PlayerUtils::PlaylistEnqueueTriggerPlay::TriggerPlay);
    }

    qDebug() << "openFiles: " << tracks.count() << " tracks loaded";
    qDebug() << "openFiles: " << targetFiles.count() << " target files loaded";
    return tracks.count() == targetFiles.count();
}

void CorPlayer::initialize() {
    initializeModels();
    initializePlayer();

    Q_EMIT initializationDone();
}

void CorPlayer::initializeModels() {
    capp->m_trackPlaylist = std::make_unique<TrackPlaylist>();
    Q_EMIT trackPlaylistChanged();

    capp->m_trackPlaylistProxyModel = std::make_unique<TrackPlaylistProxyModel>();
    capp->m_trackPlaylistProxyModel->setPlaylistModel(capp->m_trackPlaylist.get());
    Q_EMIT trackPlaylistProxyModelChanged();

    connect(this, &CorPlayer::enqueue, capp->m_trackPlaylistProxyModel.get(),
            static_cast<void (TrackPlaylistProxyModel::*)(
                const MetadataFields::EntryMetadataList &, PlayerUtils::PlaylistEnqueueMode,
                PlayerUtils::PlaylistEnqueueTriggerPlay)>(&TrackPlaylistProxyModel::enqueue));
}

void CorPlayer::initializePlayer() {
    capp->m_mediaPlayer = std::make_unique<MediaPlayerWrapper>();
    Q_EMIT mediaPlayerChanged();

    capp->m_trackManager = std::make_unique<ActiveTrackManager>();
    Q_EMIT trackManagerChanged();

    capp->m_playerManager = std::make_unique<PlayerManager>();
    Q_EMIT playerManagerChanged();

    capp->m_metadataManager = std::make_unique<TrackMetadataManager>();
    Q_EMIT metadataManagerChanged();

    capp->m_listener = std::make_unique<TracksWatchdog>();

    capp->m_trackManager->setAlbumRole(TrackPlaylist::AlbumRole);
    capp->m_trackManager->setArtistRole(TrackPlaylist::ArtistRole);
    capp->m_trackManager->setTitleRole(TrackPlaylist::TitleRole);
    capp->m_trackManager->setUrlRole(TrackPlaylist::ResourceRole);
    capp->m_trackManager->setIsPlayingRole(TrackPlaylist::IsPlayingRole);
    capp->m_trackManager->setPlaylistModel(capp->m_trackPlaylistProxyModel.get());

    // clang-format off
    connect(capp->m_trackManager.get(), &ActiveTrackManager::trackPlay, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::play);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::trackPause, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::pause);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::trackStop, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::stop);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::seek, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seek);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::saveUndoPositionInWrapper, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::saveUndoPosition);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::restoreUndoPositionInWrapper, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::restoreUndoPosition);

    connect(capp->m_trackManager.get(), &ActiveTrackManager::skipNextTrack, capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::skipNextTrack);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::sourceInError, capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::trackInError);

    connect(capp->m_trackManager.get(), &ActiveTrackManager::trackSourceChanged, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::setSource);
    connect(capp->m_trackManager.get(), &ActiveTrackManager::updateData, capp->m_trackPlaylist.get(), &TrackPlaylist::setData);

    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::ensurePlay, capp->m_trackManager.get(), &ActiveTrackManager::ensurePlay);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::requestPlay, capp->m_trackManager.get(), &ActiveTrackManager::requestPlay);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::playlistFinished, capp->m_trackManager.get(), &ActiveTrackManager::playlistFinished);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackChanged, capp->m_trackManager.get(), &ActiveTrackManager::setCurrentTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::clearPlaylistPlayer, capp->m_trackManager.get(), &ActiveTrackManager::saveForUndoClearPlaylist);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::undoClearPlaylistPlayer, capp->m_trackManager.get(), &ActiveTrackManager::restoreForUndoClearPlaylist);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::seek, capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seek);

    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::playbackStateChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackPlaybackState);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::statusChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackStatus);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::errorChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackError);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::durationChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackDuration);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::seekableChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackIsSeekable);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::positionChanged, capp->m_trackManager.get(), &ActiveTrackManager::setTrackPosition);

    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackChanged, capp->m_playerManager.get(), &PlayerManager::setCurrentTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::previousTrackChanged, capp->m_playerManager.get(), &PlayerManager::setPreviousTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::nextTrackChanged, capp->m_playerManager.get(), &PlayerManager::setNextTrack);

    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::playing, capp->m_playerManager.get(), &PlayerManager::playerPlaying);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::paused, capp->m_playerManager.get(), &PlayerManager::playerPausedOrStopped);
    connect(capp->m_mediaPlayer.get(), &MediaPlayerWrapper::stopped, capp->m_playerManager.get(), &PlayerManager::playerPausedOrStopped);

    capp->m_metadataManager->setTitleRole(TrackPlaylist::TitleRole);
    capp->m_metadataManager->setAlbumRole(TrackPlaylist::AlbumRole);
    capp->m_metadataManager->setAlbumArtistRole(TrackPlaylist::AlbumArtistRole);
    capp->m_metadataManager->setArtistRole(TrackPlaylist::ArtistRole);
    capp->m_metadataManager->setFileNameRole(TrackPlaylist::ResourceRole);
    capp->m_metadataManager->setImageRole(TrackPlaylist::ImageUrlRole);
    capp->m_metadataManager->setDatabaseIdRole(TrackPlaylist::DatabaseIdRole);
    capp->m_metadataManager->setTrackTypeRole(TrackPlaylist::ElementTypeRole);
    capp->m_metadataManager->setAlbumIdRole(TrackPlaylist::AlbumIdRole);
    capp->m_metadataManager->setIsValidRole(TrackPlaylist::IsValidAlbumArtistRole);

    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackChanged, capp->m_metadataManager.get(), &TrackMetadataManager::setCurrentTrack);
    connect(capp->m_trackPlaylistProxyModel.get(), &TrackPlaylistProxyModel::currentTrackDataChanged, capp->m_metadataManager.get(), &TrackMetadataManager::updateCurrentTrackMetadata);

    // TODO temp
    connect(capp->m_trackPlaylist.get(), &TrackPlaylist::addNewUrl, capp->m_listener.get(), &TracksWatchdog::addNewUrl);
    connect(capp->m_listener.get(), &TracksWatchdog::trackHasChanged, capp->m_trackPlaylist.get(), &TrackPlaylist::trackChanged);
    // clang-format on
}

MetadataFields::EntryMetadataList CorPlayer::sanitizePlaylist(const MetadataFields::EntryMetadataList &tracks,
                                                              const QString &workingDirectory) const {
    auto result = MetadataFields::EntryMetadataList{};
    for (const auto &track : tracks) {
        auto trackUrl = track.musicMetadata[MetadataFields::ResourceRole].toUrl();
        if (trackUrl.scheme().isEmpty() || trackUrl.isLocalFile()) {
            auto newFile = QFileInfo(trackUrl.toLocalFile());

            if (trackUrl.scheme().isEmpty()) {
                newFile = QFileInfo(trackUrl.toString());
            }

            if (newFile.isRelative()) {
                if (trackUrl.scheme().isEmpty()) {
                    newFile.setFile(workingDirectory, trackUrl.toString());
                } else {
                    newFile.setFile(workingDirectory, trackUrl.toLocalFile());
                }
            }

            if (newFile.exists()) {
                auto trackMetadata = track.musicMetadata;
                trackMetadata[MetadataFields::ResourceRole] = QUrl::fromLocalFile(newFile.absoluteFilePath());
                result.push_back({trackMetadata, track.title, {}});
            }
        } else {
            result.push_back(track);
        }
    }
    return result;
}

TrackPlaylist *CorPlayer::trackPlaylist() const {
    return capp->m_trackPlaylist.get();
}

TrackPlaylistProxyModel *CorPlayer::trackPlaylistProxyModel() const {
    return capp->m_trackPlaylistProxyModel.get();
}

MediaPlayerWrapper *CorPlayer::mediaPlayer() const {
    return capp->m_mediaPlayer.get();
}

ActiveTrackManager *CorPlayer::trackManager() const {
    return capp->m_trackManager.get();
}

PlayerManager *CorPlayer::playerManager() const {
    return capp->m_playerManager.get();
}

TrackMetadataManager *CorPlayer::metadataManager() const {
    return capp->m_metadataManager.get();
}

#include "moc_corplayer.cpp" // TODO check why necessary
