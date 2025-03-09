#include "activetrackmanager.h"

#include "models/playlistmodel.hpp"

#include <QDateTime>
#include <QTimer>

TrackMetadata::TrackMetadata(QObject* parent) : QObject(parent) {}

QString TrackMetadata::title() const {
    if (!m_currentTrack.isValid()) return {};

    const QVariant data = m_currentTrack.data(m_roles.title);
    return data.isValid() ? data.toString() : QString();
}

QString TrackMetadata::artist() const {
    if (!m_currentTrack.isValid()) return {};

    QVariant data = m_currentTrack.data(m_roles.artist);
    if (!data.isValid() || data.toString().isEmpty()) {
        data = m_currentTrack.data(m_roles.albumArtist);
    }

    return data.isValid() ? data.toString() : QString();
}

QString TrackMetadata::album() const {
    if (!m_currentTrack.isValid()) return {};

    const QVariant data = m_currentTrack.data(m_roles.album);
    return data.isValid() ? data.toString() : QString();
}

QString TrackMetadata::albumArtist() const {
    if (!m_currentTrack.isValid()) return {};

    const QVariant data = m_currentTrack.data(m_roles.albumArtist);
    return data.isValid() ? data.toString() : QString();
}

QUrl TrackMetadata::fileUrl() const {
    if (!m_currentTrack.isValid()) return {};

    const QVariant data = m_currentTrack.data(m_roles.fileUrl);
    return data.isValid() ? data.toUrl() : QUrl();
}

QUrl TrackMetadata::coverUrl() const {
    if (!m_currentTrack.isValid()) return {};

    const QVariant data = m_currentTrack.data(m_roles.coverUrl);
    return data.isValid() ? data.toUrl() : QUrl();
}

quint64 TrackMetadata::databaseId() const {
    if (!m_currentTrack.isValid()) return 0;

    const QVariant data = m_currentTrack.data(m_roles.databaseId);
    return data.isValid() ? data.toULongLong() : 0;
}

PlayerUtils::PlaylistEntryType TrackMetadata::elementType() const {
    if (!m_currentTrack.isValid()) return PlayerUtils::PlaylistEntryType::Unknown;

    const QVariant data = m_currentTrack.data(m_roles.elementType);
    return data.isValid() ? static_cast<PlayerUtils::PlaylistEntryType>(data.toInt())
                          : PlayerUtils::PlaylistEntryType::Unknown;
}

bool TrackMetadata::isPlaying() const {
    if (!m_currentTrack.isValid()) return false;

    const QVariant data = m_currentTrack.data(m_roles.isPlaying);
    return data.isValid() ? data.toBool() : false;
}

quint64 TrackMetadata::albumId() const {
    if (!m_currentTrack.isValid()) return 0;

    const QVariant data = m_currentTrack.data(m_roles.albumId);
    return data.isValid() ? data.toULongLong() : 0;
}

bool TrackMetadata::isValid() const {
    return m_currentTrack.isValid() && m_currentTrack.data(m_roles.isValid).toBool();
}

void TrackMetadata::setRoles(const int titleRole, const int artistRole, const int albumRole, const int albumArtistRole,
                             const int fileUrlRole, const int coverUrlRole, const int databaseIdRole,
                             const int elementTypeRole, const int isPlayingRole, const int albumIdRole,
                             const int isValidRole) {
    m_roles.title = titleRole;
    m_roles.artist = artistRole;
    m_roles.album = albumRole;
    m_roles.albumArtist = albumArtistRole;
    m_roles.fileUrl = fileUrlRole;
    m_roles.coverUrl = coverUrlRole;
    m_roles.databaseId = databaseIdRole;
    m_roles.elementType = elementTypeRole;
    m_roles.isPlaying = isPlayingRole;
    m_roles.albumId = albumIdRole;
    m_roles.isValid = isValidRole;
}

void TrackMetadata::setCurrentTrack(const QPersistentModelIndex& currentTrack) {
    if (m_currentTrack == currentTrack) return;

    m_currentTrack = currentTrack;
    updateMetadata();
}

void TrackMetadata::updateMetadata() {
    const QVariant newTitle = m_currentTrack.data(m_roles.title);
    if (m_metadata.title != newTitle) {
        m_metadata.title = newTitle.toString();
        Q_EMIT titleChanged();
    }

    const QVariant newArtist = m_currentTrack.data(m_roles.artist);
    if (m_metadata.artist != newArtist) {
        m_metadata.artist = newArtist.toString();
        Q_EMIT artistChanged();
    }

    const QVariant newAlbum = m_currentTrack.data(m_roles.album);
    if (m_metadata.album != newAlbum) {
        m_metadata.album = newAlbum.toString();
        Q_EMIT albumChanged();
    }

    const QVariant newAlbumArtist = m_currentTrack.data(m_roles.albumArtist);
    if (m_metadata.albumArtist != newAlbumArtist) {
        m_metadata.albumArtist = newAlbumArtist.toString();
        Q_EMIT albumArtistChanged();
    }

    const QVariant newFileUrl = m_currentTrack.data(m_roles.fileUrl);
    if (m_metadata.fileUrl != newFileUrl) {
        m_metadata.fileUrl = newFileUrl.toUrl();
        Q_EMIT fileUrlChanged();
    }

    const QVariant newCoverUrl = m_currentTrack.data(m_roles.coverUrl);
    if (m_metadata.coverUrl != newCoverUrl) {
        m_metadata.coverUrl = newCoverUrl.toUrl();
        Q_EMIT coverUrlChanged();
    }

    {
        bool conversionOk = false;
        const quint64 newDatabaseId = m_currentTrack.data(m_roles.databaseId).toULongLong(&conversionOk);
        if (conversionOk && m_metadata.databaseId != newDatabaseId) {
            m_metadata.databaseId = newDatabaseId;
            Q_EMIT databaseIdChanged();
        } else if (!conversionOk && m_metadata.databaseId != 0) {
            m_metadata.databaseId = 0;
            Q_EMIT databaseIdChanged();
        }
    }

    const QVariant newElementType = m_currentTrack.data(m_roles.elementType);
    if (m_metadata.elementType != newElementType) {
        m_metadata.elementType = static_cast<PlayerUtils::PlaylistEntryType>(newElementType.toInt());
        Q_EMIT elementTypeChanged();
    }

    const QVariant newIsPlaying = m_currentTrack.data(m_roles.isPlaying);
    if (m_metadata.isPlaying != newIsPlaying) {
        m_metadata.isPlaying = newIsPlaying.toBool();
        Q_EMIT isPlayingChanged();
    }

    bool conversionOk = false;
    const quint64 newAlbumId = m_currentTrack.data(m_roles.albumId).toULongLong(&conversionOk);
    if (conversionOk && m_metadata.albumId != newAlbumId) {
        m_metadata.albumId = newAlbumId;
        Q_EMIT albumIdChanged();
    } else if (!conversionOk && m_metadata.albumId != 0) {
        m_metadata.albumId = 0;
        Q_EMIT albumIdChanged();
    }

    const QVariant newIsValid = m_currentTrack.data(m_roles.isValid);
    if (m_metadata.isValid != newIsValid) {
        m_metadata.isValid = newIsValid.toBool();
        Q_EMIT isValidChanged();
    }
}

ActiveTrackManager::ActiveTrackManager(QObject* parent)
    : QObject(parent), m_trackMetadata(std::make_unique<TrackMetadata>()) {
    m_trackMetadata->setRoles(Metadata::Fields::Title, Metadata::Fields::Artist, Metadata::Fields::Album,
                              Metadata::Fields::AlbumArtist, Metadata::Fields::ResourceUrl,
                              Metadata::Fields::CoverImage, Metadata::Fields::DatabaseId, Metadata::Fields::ElementType,
                              Metadata::PlaylistFields::IsPlaying, Metadata::Fields::AlbumId,
                              Metadata::Fields::IsValid);
}

TrackMetadata* ActiveTrackManager::trackMetadata() const {
    return m_trackMetadata.get();
}

QPersistentModelIndex ActiveTrackManager::currentTrack() const {
    return m_currentTrack;
}

QAbstractItemModel* ActiveTrackManager::playlistModel() const {
    return m_playlistModel;
}

QUrl ActiveTrackManager::trackSource() const {
    if (!m_currentTrack.isValid()) return {};
    return m_currentTrack.data(m_trackMetadata->m_roles.fileUrl).toUrl();
}

QMediaPlayer::MediaStatus ActiveTrackManager::mediaStatus() const {
    return m_mediaStatus;
}

QMediaPlayer::PlaybackState ActiveTrackManager::playbackState() const {
    return m_playbackState;
}

QMediaPlayer::Error ActiveTrackManager::trackError() const {
    return m_trackError;
}

qint64 ActiveTrackManager::duration() const {
    return m_duration;
}

bool ActiveTrackManager::seekable() const {
    return m_seekable;
}

qint64 ActiveTrackManager::position() const {
    return m_position;
}

qint64 ActiveTrackManager::trackControlPosition() const {
    return m_position;
}

QVariantMap ActiveTrackManager::persistentState() const {
    auto persistentState = QVariantMap();

    persistentState[QStringLiteral("playingState")] = m_isPlaying;

    persistentState[QStringLiteral("activePosition")] = m_position;
    persistentState[QStringLiteral("activeDduration")] = m_duration;

    if (m_currentTrack.isValid()) {
        const auto& roles = m_trackMetadata->m_roles;
        persistentState[QStringLiteral("activeTrackTitle")] = m_currentTrack.data(roles.title);
        persistentState[QStringLiteral("activeTrackArtist")] = m_currentTrack.data(roles.artist);
        persistentState[QStringLiteral("activeTrackAlbum")] = m_currentTrack.data(roles.album);
    } else {
        persistentState[QStringLiteral("activeTrackTitle")] = {};
        persistentState[QStringLiteral("activeTrackArtist")] = {};
        persistentState[QStringLiteral("activeTrackAlbum")] = {};
    }

    return persistentState;
}

void ActiveTrackManager::setCurrentTrack(const QPersistentModelIndex& currentTrack) {
    m_previousTrack = m_currentTrack;
    m_currentTrack = currentTrack;

    if (m_currentTrack.isValid()) {
        restorePreviousState();
    }

    m_trackError = QMediaPlayer::NoError;

    if (m_previousTrack != m_currentTrack || m_isPlaying) {
        Q_EMIT currentTrackChanged();
        if (m_currentTrack.isValid()) {
            Q_EMIT trackSourceChanged(m_currentTrack.data(m_trackMetadata->m_roles.fileUrl).toUrl());
        }
    }

    if (m_playbackState == QMediaPlayer::PlayingState || m_playbackState == QMediaPlayer::PausedState) {
        enqueue([this] {
            Q_EMIT stopTrack();
        });
        if (m_isPlaying && !m_currentTrack.isValid()) {
            m_isPlaying = false;
        }
        m_skippingCurrentTrack = true;
    }
}

void ActiveTrackManager::saveForUndoClearPlaylist() {
    m_undoPlayingState = m_isPlaying;
    m_undoTrackPosition = m_position;
    Q_EMIT saveUndoPositionInWrapper(m_undoTrackPosition);
}

void ActiveTrackManager::restoreForUndoClearPlaylist() {
    m_position = m_undoTrackPosition;
    Q_EMIT seek(m_position);

    m_isPlaying = m_undoPlayingState;
    Q_EMIT restoreUndoPositionInWrapper();
}

void ActiveTrackManager::setPlaylistModel(QAbstractItemModel* playlistModel) {
    if (m_playlistModel == playlistModel) return;

    if (m_playlistModel != nullptr) {
        disconnect(m_playlistModel, nullptr, this, nullptr);
    }

    m_playlistModel = playlistModel;

    if (m_playlistModel != nullptr) {
        connect(m_playlistModel, &QAbstractItemModel::dataChanged, this, &ActiveTrackManager::tracksDataChanged);
    }

    Q_EMIT playlistModelChanged();
}

void ActiveTrackManager::setMediaStatus(const QMediaPlayer::MediaStatus mediaStatus) {
    if (m_mediaStatus == mediaStatus) return;

    m_mediaStatus = mediaStatus;
    Q_EMIT mediaStatusChanged();

    switch (m_mediaStatus) {
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadingMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::StalledMedia:
    case QMediaPlayer::BufferedMedia:
    case QMediaPlayer::EndOfMedia:
        break;
    case QMediaPlayer::LoadedMedia:
        if (m_isPlaying) {
            enqueue([this] {
                Q_EMIT playTrack();
            });
        }
        break;
    case QMediaPlayer::InvalidMedia:
        enqueue([this, reason = PlayerUtils::SkipReason::Automatic] {
            Q_EMIT skipNextTrack(reason);
        });
        break;
    }
}

void ActiveTrackManager::setPlaybackState(const QMediaPlayer::PlaybackState playbackState) {
    if (m_playbackState == playbackState) return;

    m_playbackState = playbackState;
    Q_EMIT playbackStateChanged();

    const auto& roles = m_trackMetadata->m_roles;

    if (!m_skippingCurrentTrack) {
        switch (m_playbackState) {
        case QMediaPlayer::StoppedState:
            if (m_mediaStatus == QMediaPlayer::EndOfMedia) {
                Q_EMIT trackFinishedPlaying(m_currentTrack.data(roles.fileUrl).toUrl(), QDateTime::currentDateTime());
            }
            if (m_mediaStatus == QMediaPlayer::EndOfMedia || m_mediaStatus == QMediaPlayer::InvalidMedia) {
                enqueue([this, reason = PlayerUtils::SkipReason::Automatic] {
                    Q_EMIT skipNextTrack(reason);
                });
            }
            if (m_playlistModel != nullptr && m_currentTrack.isValid()) {
                m_playlistModel->setData(m_currentTrack, PlaylistModel::NotPlaying, roles.isPlaying);
            }
            break;
        case QMediaPlayer::PlayingState:
            if (m_playlistModel != nullptr && m_currentTrack.isValid()) {
                m_playlistModel->setData(m_currentTrack, PlaylistModel::IsPlaying, roles.isPlaying);
                Q_EMIT trackStartedPlaying(m_currentTrack.data(roles.fileUrl).toUrl(), QDateTime::currentDateTime());
            }
            break;
        case QMediaPlayer::PausedState:
            if (m_playlistModel != nullptr && m_currentTrack.isValid()) {
                m_playlistModel->setData(m_currentTrack, PlaylistModel::IsPaused, roles.isPlaying);
            }
            break;
        }
    } else {
        switch (m_playbackState) {
        case QMediaPlayer::StoppedState:
            notifyTrackSourceProperty();
            m_skippingCurrentTrack = false;
            if (m_playlistModel != nullptr && m_previousTrack.isValid()) {
                m_playlistModel->setData(m_previousTrack, PlaylistModel::NotPlaying, roles.isPlaying);
            }
            break;
        case QMediaPlayer::PlayingState:
            if (m_playlistModel != nullptr && m_currentTrack.isValid()) {
                m_playlistModel->setData(m_currentTrack, PlaylistModel::IsPlaying, roles.isPlaying);
                Q_EMIT trackStartedPlaying(m_currentTrack.data(roles.fileUrl).toUrl(), QDateTime::currentDateTime());
            }
            break;
        case QMediaPlayer::PausedState:
            if (m_playlistModel != nullptr && m_currentTrack.isValid()) {
                m_playlistModel->setData(m_currentTrack, PlaylistModel::IsPaused, roles.isPlaying);
            }
            break;
        }
    }
}

void ActiveTrackManager::setTrackError(const QMediaPlayer::Error trackError) {
    if (m_trackError == trackError) return;

    m_trackError = trackError;
    Q_EMIT trackErrorChanged();

    if (m_trackError != QMediaPlayer::NoError) {
        const auto currentSource = trackSource();

        Q_EMIT sourceInError(currentSource, m_trackError);

        if (currentSource.isLocalFile()) {
            Q_EMIT displayTrackError(currentSource.toLocalFile());
        } else {
            Q_EMIT displayTrackError(currentSource.toString());
        }
    }
}

void ActiveTrackManager::ensurePause() {
    if (m_isPlaying) {
        m_isPlaying = false;
        enqueue([this] {
            Q_EMIT pauseTrack();
        });
    }
}

void ActiveTrackManager::ensurePlay() {
    if (!m_isPlaying) {
        m_isPlaying = true;
        enqueue([this] {
            Q_EMIT playTrack();
        });
    }
}

void ActiveTrackManager::requestPlay() {
    m_isPlaying = true;
}

void ActiveTrackManager::stop() {
    m_isPlaying = false;
    enqueue([this] {
        Q_EMIT stopTrack();
    });
}

void ActiveTrackManager::playPause() {
    m_isPlaying = !m_isPlaying;

    switch (m_mediaStatus) {
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
    case QMediaPlayer::LoadingMedia:
        if (m_isPlaying) {
            enqueue([this] {
                Q_EMIT playTrack();
            });
        } else {
            enqueue([this] {
                Q_EMIT pauseTrack();
            });
        }
        break;
    case QMediaPlayer::EndOfMedia:
        if (m_playbackState == QMediaPlayer::PlayingState && !m_isPlaying) {
            enqueue([this] {
                Q_EMIT pauseTrack();
            });
        } else if (m_playbackState == QMediaPlayer::PausedState && m_isPlaying) {
            enqueue([this] {
                Q_EMIT playTrack();
            });
        }
        break;
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::StalledMedia:
    case QMediaPlayer::InvalidMedia:
        break;
    }
}

void ActiveTrackManager::setDuration(const qint64 duration) {
    if (m_duration == duration) return;

    m_duration = duration;
    Q_EMIT durationChanged();
}

void ActiveTrackManager::setSeekable(const bool seekable) {
    if (m_seekable == seekable) return;

    m_seekable = seekable;
    Q_EMIT seekableChanged();
}

void ActiveTrackManager::setPosition(const qint64 position) {
    if (m_position == position) return;

    m_position = position;
    Q_EMIT positionChanged();

    QTimer::singleShot(0, this, [this]() {
        Q_EMIT trackControlPositionChanged();
    });
}

void ActiveTrackManager::setTrackControlPosition(const int trackControlPosition) {
    Q_EMIT seek(trackControlPosition);
}

void ActiveTrackManager::setPersistentState(const QVariantMap& persistentState) {
    if (m_persistentState == persistentState) return;

    m_persistentState = persistentState;
    Q_EMIT persistentStateChanged();

    if (m_currentTrack.isValid()) {
        restorePreviousState();
    }
}

void ActiveTrackManager::trackSeek(const int newPosition) {
    Q_EMIT seek(newPosition);
}

void ActiveTrackManager::playlistFinished() {
    m_isPlaying = false;
}

void ActiveTrackManager::tracksDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                           const QList<int>& roles) {
    if (!m_currentTrack.isValid()) return;
    if (m_currentTrack.row() > bottomRight.row() || m_currentTrack.row() < topLeft.row()) return;
    if (m_currentTrack.column() > bottomRight.column() || m_currentTrack.column() < topLeft.column()) return;

    if (roles.isEmpty()) {
        notifyTrackSourceProperty();
        restorePreviousState();
    } else {
        for (const auto role : roles) {
            if (role == m_trackMetadata->m_roles.fileUrl) {
                notifyTrackSourceProperty();
                restorePreviousState();
            }
        }
    }
}

void ActiveTrackManager::notifyTrackSourceProperty() {
    auto urlValue = m_currentTrack.data(m_trackMetadata->m_roles.fileUrl).toUrl();

    if (m_skippingCurrentTrack || m_previousTrackSource.toUrl() != urlValue) {
        Q_EMIT trackSourceChanged(urlValue);
        m_previousTrackSource = urlValue;
    }
}

void ActiveTrackManager::restorePreviousState() {
    if (m_persistentState.isEmpty()) return;

    const auto itTitle = m_persistentState.find(QStringLiteral("activeTrackTitle"));
    const auto itArtistName = m_persistentState.find(QStringLiteral("activeTrackArtist"));
    const auto itAlbumName = m_persistentState.find(QStringLiteral("activeTrackAlbum"));

    if (itTitle == m_persistentState.end() || itArtistName == m_persistentState.end() ||
        itAlbumName == m_persistentState.end()) {
        return;
    }

    const auto& roles = m_trackMetadata->m_roles;
    if (*itTitle != m_currentTrack.data(roles.title) ||
        (itArtistName->isValid() && *itArtistName != m_currentTrack.data(roles.artist)) ||
        (itAlbumName->isValid() && *itAlbumName != m_currentTrack.data(roles.album))) {
        if (m_currentTrack.isValid() && m_currentTrack.data(roles.title).isValid() &&
            m_currentTrack.data(roles.artist).isValid() && m_currentTrack.data(roles.album).isValid()) {
            m_persistentState.clear();
        }
        return;
    }

    if (!m_currentTrack.data(roles.fileUrl).toUrl().isValid()) return;

    auto playerPosition = m_persistentState.find(QStringLiteral("trackPosition"));
    if (playerPosition != m_persistentState.end()) {
        setDuration(playerPosition->toLongLong());
        Q_EMIT seek(m_position);
    }
    auto playerDuration = m_persistentState.find(QStringLiteral("trackDuration"));
    if (playerDuration != m_persistentState.end()) {
        setDuration(playerDuration->toInt());
    }

    m_persistentState.clear();
}
