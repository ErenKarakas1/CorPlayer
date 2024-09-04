#include "activetrackmanager.h"

#include "playlist/trackplaylist.h"

#include <QTimer>
#include <QDateTime>

ActiveTrackManager::ActiveTrackManager(QObject *parent) : QObject(parent) {}

QPersistentModelIndex ActiveTrackManager::currentTrack() const {
    return m_currentTrack;
}

QAbstractItemModel* ActiveTrackManager::playlistModel() const {
    return m_playlistModel;
}

int ActiveTrackManager::titleRole() const {
    return m_titleRole;
}

int ActiveTrackManager::artistRole() const {
    return m_artistRole;
}

int ActiveTrackManager::albumRole() const {
    return m_albumRole;
}

int ActiveTrackManager::urlRole() const {
    return m_urlRole;
}

int ActiveTrackManager::isPlayingRole() const {
    return m_isPlayingRole;
}

QUrl ActiveTrackManager::trackSource() const {
    if (!m_currentTrack.isValid()) {
        return {};
    }

    return m_currentTrack.data(m_urlRole).toUrl();
}

QMediaPlayer::MediaStatus ActiveTrackManager::trackStatus() const {
    return m_trackStatus;
}

QMediaPlayer::PlaybackState ActiveTrackManager::trackPlaybackState() const {
    return m_trackPlaybackState;
}

QMediaPlayer::Error ActiveTrackManager::trackError() const {
    return m_trackError;
}

qint64 ActiveTrackManager::trackDuration() const {
    return m_trackDuration;
}

bool ActiveTrackManager::trackIsSeekable() const {
    return m_trackIsSeekable;
}

qint64 ActiveTrackManager::trackPosition() const {
    return m_trackPosition;
}

qint64 ActiveTrackManager::trackControlPosition() const {
    return m_trackPosition;
}

QVariantMap ActiveTrackManager::persistentState() const {
    auto persistentStateValue = QVariantMap();

    persistentStateValue[QStringLiteral("playingState")] = m_isPlaying;

    persistentStateValue[QStringLiteral("trackPosition")] = m_trackPosition;
    persistentStateValue[QStringLiteral("trackDuration")] = m_trackDuration;

    if (m_currentTrack.isValid()) {
        persistentStateValue[QStringLiteral("activeTrackTitle")] = m_currentTrack.data(m_titleRole);
        persistentStateValue[QStringLiteral("activeTrackArtist")] = m_currentTrack.data(m_artistRole);
        persistentStateValue[QStringLiteral("activeTrackAlbum")] = m_currentTrack.data(m_albumRole);
    } else {
        persistentStateValue[QStringLiteral("activeTrackTitle")] = {};
        persistentStateValue[QStringLiteral("activeTrackArtist")] = {};
        persistentStateValue[QStringLiteral("activeTrackAlbum")] = {};
    }

    return persistentStateValue;
}

void ActiveTrackManager::setCurrentTrack(const QPersistentModelIndex &newCurrentTrack) {
    m_previousTrack = m_currentTrack;
    m_currentTrack = newCurrentTrack;

    if (m_currentTrack.isValid()) {
        restorePreviousState();
    }

    m_trackError = QMediaPlayer::NoError;

    if (m_previousTrack != m_currentTrack || m_isPlaying) {
        Q_EMIT currentTrackChanged();
    }

    switch (m_trackPlaybackState) {
        case QMediaPlayer::StoppedState:
            Q_EMIT trackSourceChanged(m_currentTrack.data(m_urlRole).toUrl());
            break;
        case QMediaPlayer::PlayingState:
        case QMediaPlayer::PausedState:
            triggerStop();
            if (m_isPlaying && !m_currentTrack.isValid()) {
                m_isPlaying = false;
            }
            m_skippingCurrentTrack = true;
            break;
    }
}

void ActiveTrackManager::saveForUndoClearPlaylist() {
    m_undoPlayingState = m_isPlaying;
    m_undoTrackPosition = m_trackPosition;
    Q_EMIT saveUndoPositionInWrapper(m_undoTrackPosition);
}

void ActiveTrackManager::restoreForUndoClearPlaylist() {
    m_trackPosition = m_undoTrackPosition;
    Q_EMIT seek(m_trackPosition);

    m_isPlaying = m_undoPlayingState;
    Q_EMIT restoreUndoPositionInWrapper();
}

void ActiveTrackManager::setPlaylistModel(QAbstractItemModel *newPlaylistModel) {
    if (m_playlistModel == newPlaylistModel) {
        return;
    }

    if (m_playlistModel) {
        disconnect(m_playlistModel, &QAbstractItemModel::dataChanged, this, &ActiveTrackManager::tracksDataChanged);
    }

    m_playlistModel = newPlaylistModel;

    if (m_playlistModel) {
        connect(m_playlistModel, &QAbstractItemModel::dataChanged, this, &ActiveTrackManager::tracksDataChanged);
    }

    Q_EMIT playlistModelChanged();
}

void ActiveTrackManager::setTitleRole(int newTitleRole) {
    if (m_titleRole == newTitleRole) {
        return;
    }

    m_titleRole = newTitleRole;
    Q_EMIT titleRoleChanged();

    if (m_currentTrack.isValid()) {
        restorePreviousState();
    }
}

void ActiveTrackManager::setArtistRole(int newArtistRole) {
    if (m_artistRole == newArtistRole) {
        return;
    }

    m_artistRole = newArtistRole;
    Q_EMIT artistRoleChanged();

    if (m_currentTrack.isValid()) {
        restorePreviousState();
    }
}

void ActiveTrackManager::setAlbumRole(int newAlbumRole) {
    if (m_albumRole == newAlbumRole) {
        return;
    }

    m_albumRole = newAlbumRole;
    Q_EMIT albumRoleChanged();

    if (m_currentTrack.isValid()) {
        restorePreviousState();
    }
}

void ActiveTrackManager::setUrlRole(int newUrlRole) {
    m_urlRole = newUrlRole;
    Q_EMIT urlRoleChanged();

    notifyTrackSourceProperty();
    restorePreviousState();
}

void ActiveTrackManager::setIsPlayingRole(int newIsPlayingRole) {
    if (m_isPlayingRole == newIsPlayingRole) {
        return;
    }

    m_isPlayingRole = newIsPlayingRole;
    Q_EMIT isPlayingRoleChanged();
}

void ActiveTrackManager::setTrackStatus(QMediaPlayer::MediaStatus newTrackStatus) {
    if (m_trackStatus == newTrackStatus) {
        return;
    }

    m_trackStatus = newTrackStatus;
    Q_EMIT trackStatusChanged();

    switch (m_trackStatus) {
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::LoadingMedia:
        case QMediaPlayer::BufferingMedia:
        case QMediaPlayer::StalledMedia:
        case QMediaPlayer::BufferedMedia:
        case QMediaPlayer::EndOfMedia:
            break;
        case QMediaPlayer::LoadedMedia:
            if (m_isPlaying) {
                triggerPlay();
            }
            break;
        case QMediaPlayer::InvalidMedia:
            triggerSkipNextTrack();
            break;
    }
}

void ActiveTrackManager::setTrackPlaybackState(QMediaPlayer::PlaybackState newTrackPlaybackState) {
    if (m_trackPlaybackState == newTrackPlaybackState) {
        return;
    }

    m_trackPlaybackState = newTrackPlaybackState;
    Q_EMIT trackPlaybackStateChanged();

    if (!m_skippingCurrentTrack) {
        switch (m_trackPlaybackState) {
            case QMediaPlayer::StoppedState:
                if (m_trackStatus == QMediaPlayer::EndOfMedia) {
                    Q_EMIT trackFinishedPlaying(m_currentTrack.data(m_urlRole).toUrl(), QDateTime::currentDateTime());
                }
                if (m_trackStatus == QMediaPlayer::EndOfMedia || m_trackStatus == QMediaPlayer::InvalidMedia) {
                    triggerSkipNextTrack();
                }
                if (m_playlistModel && m_currentTrack.isValid()) {
                    m_playlistModel->setData(m_currentTrack, TrackPlaylist::NotPlaying, m_isPlayingRole);
                }
                break;
            case QMediaPlayer::PlayingState:
                if (m_playlistModel && m_currentTrack.isValid()) {
                    m_playlistModel->setData(m_currentTrack, TrackPlaylist::IsPlaying, m_isPlayingRole);
                    Q_EMIT trackStartedPlaying(m_currentTrack.data(m_urlRole).toUrl(), QDateTime::currentDateTime());
                }
                break;
            case QMediaPlayer::PausedState:
                if (m_playlistModel && m_currentTrack.isValid()) {
                    m_playlistModel->setData(m_currentTrack, TrackPlaylist::IsPaused, m_isPlayingRole);
                }
                break;
        }
    }
    else {
        switch (m_trackPlaybackState) {
            case QMediaPlayer::StoppedState:
                notifyTrackSourceProperty();
                m_skippingCurrentTrack = false;
                if (m_playlistModel && m_previousTrack.isValid()) {
                    m_playlistModel->setData(m_previousTrack, TrackPlaylist::NotPlaying, m_isPlayingRole);
                }
                break;
            case QMediaPlayer::PlayingState:
                if (m_playlistModel && m_currentTrack.isValid()) {
                    m_playlistModel->setData(m_currentTrack, TrackPlaylist::IsPlaying, m_isPlayingRole);
                    Q_EMIT trackStartedPlaying(m_currentTrack.data(m_urlRole).toUrl(), QDateTime::currentDateTime());
                }
                break;
            case QMediaPlayer::PausedState:
                if (m_playlistModel && m_currentTrack.isValid()) {
                    m_playlistModel->setData(m_currentTrack, TrackPlaylist::IsPaused, m_isPlayingRole);
                }
                break;
        }
    }
}

void ActiveTrackManager::setTrackError(QMediaPlayer::Error newTrackError) {
    if (m_trackError == newTrackError) {
        return;
    }

    m_trackError = newTrackError;
    Q_EMIT trackErrorChanged();

    if (m_trackError != QMediaPlayer::NoError) {
        auto currentSource = trackSource();

        Q_EMIT sourceInError(currentSource, m_trackError);

        if (currentSource.isLocalFile()) {
            Q_EMIT displayTrackError(currentSource.toLocalFile());
        }
        else {
            Q_EMIT displayTrackError(currentSource.toString());
        }
    }
}

void ActiveTrackManager::ensurePause() {
    if (m_isPlaying) {
        m_isPlaying = false;
        triggerPause();
    }
}

void ActiveTrackManager::ensurePlay() {
    if (!m_isPlaying) {
        m_isPlaying = true;
        triggerPlay();
    }
}

void ActiveTrackManager::requestPlay() {
    qDebug() << "ActiveTrackManager::requestPlay()";
    m_isPlaying = true;
}

void ActiveTrackManager::stop() {
    m_isPlaying = false;
    triggerStop();
}

void ActiveTrackManager::playPause() {
    m_isPlaying = !m_isPlaying;
    qDebug() << "ActiveTrackManager::playPause()" << m_trackStatus << m_isPlaying;

    switch (m_trackStatus) {
        case QMediaPlayer::LoadedMedia:
        case QMediaPlayer::BufferingMedia:
        case QMediaPlayer::BufferedMedia:
        case QMediaPlayer::LoadingMedia:
            if (m_isPlaying) {
                triggerPlay();
                qDebug() << "ActiveTrackManager::playPause() - triggerPlay()";
            }
            else {
                triggerPause();
                qDebug() << "ActiveTrackManager::playPause() - triggerPause()";
            }
            break;
        case QMediaPlayer::EndOfMedia:
            if (m_trackPlaybackState == QMediaPlayer::PlayingState && !m_isPlaying) {
                triggerPause();
            }
            else if (m_trackPlaybackState == QMediaPlayer::PausedState && m_isPlaying) {
                triggerPlay();
            }
            break;
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::StalledMedia:
        case QMediaPlayer::InvalidMedia:
            break;
    }
}

void ActiveTrackManager::setTrackDuration(qint64 newTrackDuration) {
    if (m_trackDuration == newTrackDuration) {
        return;
    }

    m_trackDuration = newTrackDuration;
    Q_EMIT trackDurationChanged();
}

void ActiveTrackManager::setTrackIsSeekable(bool newTrackIsSeekable) {
    if (m_trackIsSeekable == newTrackIsSeekable) {
        return;
    }

    m_trackIsSeekable = newTrackIsSeekable;
    Q_EMIT trackIsSeekableChanged();
}

void ActiveTrackManager::setTrackPosition(qint64 newTrackPosition) {
    if (m_trackPosition == newTrackPosition) {
        return;
    }

    m_trackPosition = newTrackPosition;
    Q_EMIT trackPositionChanged();
    QTimer::singleShot(0, this, [this]() {
        Q_EMIT trackControlPositionChanged();
    });
}

void ActiveTrackManager::setTrackControlPosition(int newTrackControlPosition) {
    Q_EMIT seek(newTrackControlPosition);
}

void ActiveTrackManager::setPersistentState(const QVariantMap &newPersistentState) {
    if (m_persistentState == newPersistentState) {
        return;
    }

    m_persistentState = newPersistentState;

    Q_EMIT persistentStateChanged();

    if (m_currentTrack.isValid()) {
        restorePreviousState();
    }
}

void ActiveTrackManager::trackSeek(int newPosition) {
    Q_EMIT seek(newPosition);
}

void ActiveTrackManager::playlistFinished() {
    m_isPlaying = false;
}

void ActiveTrackManager::tracksDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                          const QList<int> &roles) {

    if (!m_currentTrack.isValid()) {
        return;
    }

    if (m_currentTrack.row() > bottomRight.row() || m_currentTrack.row() < topLeft.row()) {
        return;
    }

    if (m_currentTrack.column() > bottomRight.column() || m_currentTrack.column() < topLeft.column()) {
        return;
    }

    if (roles.isEmpty()) {
        notifyTrackSourceProperty();
        restorePreviousState();
    }
    else {
        for (auto oneRole : roles) {
            if (oneRole == m_urlRole) {
                notifyTrackSourceProperty();
                restorePreviousState();
            }
        }
    }
}

void ActiveTrackManager::notifyTrackSourceProperty() {
    auto newUrlValue = m_currentTrack.data(m_urlRole);
    if (m_skippingCurrentTrack || m_previousTrackSource != newUrlValue) {
        Q_EMIT trackSourceChanged(m_currentTrack.data(m_urlRole).toUrl());

        m_previousTrackSource = newUrlValue;
    }
}

void ActiveTrackManager::triggerPlay() {
    QTimer::singleShot(0, this, [this]() {
        Q_EMIT trackPlay();
    });
}

void ActiveTrackManager::triggerPause() {
    QTimer::singleShot(0, this, [this]() {
        Q_EMIT trackPause();
    });
}

void ActiveTrackManager::triggerStop() {
    QTimer::singleShot(0, this, [this]() {
        Q_EMIT trackStop();
    });
}

void ActiveTrackManager::triggerSkipNextTrack(PlayerUtils::SkipReason reason /*= SkipReason::Automatic */) {
    QTimer::singleShot(0, this, [this, reason]() {
        Q_EMIT skipNextTrack(reason);
    });
}

void ActiveTrackManager::restorePreviousState() {
    if (m_persistentState.isEmpty()) {
        return;
    }

    auto itTitle = m_persistentState.find(QStringLiteral("activeTrackTitle"));
    auto itArtistName = m_persistentState.find(QStringLiteral("activeTrackArtist"));
    auto itAlbumName = m_persistentState.find(QStringLiteral("activeTrackAlbum"));

    if (itTitle == m_persistentState.end() || itArtistName == m_persistentState.end() ||
        itAlbumName == m_persistentState.end()) {
        return;
    }

    if (*itTitle != m_currentTrack.data(m_titleRole) ||
        (itArtistName->isValid() && *itArtistName != m_currentTrack.data(m_artistRole)) ||
        (itAlbumName->isValid() && *itAlbumName != m_currentTrack.data(m_albumRole))) {
        if (m_currentTrack.isValid() && m_currentTrack.data(m_titleRole).isValid() && m_currentTrack.data(m_artistRole).
            isValid() &&
            m_currentTrack.data(m_albumRole).isValid()) {
            m_persistentState.clear();
        }

        return;
    }

    if (!m_currentTrack.data(m_urlRole).toUrl().isValid()) {
        return;
    }

    auto playerPosition = m_persistentState.find(QStringLiteral("trackPosition"));
    if (playerPosition != m_persistentState.end()) {
        setTrackDuration(playerPosition->toLongLong());
        Q_EMIT seek(m_trackPosition);
    }
    auto playerDuration = m_persistentState.find(QStringLiteral("trackDuration"));
    if (playerDuration != m_persistentState.end()) {
        setTrackDuration(playerDuration->toInt());
    }

    m_persistentState.clear();
}
