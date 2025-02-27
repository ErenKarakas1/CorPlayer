#include "playermanager.h"

PlayerManager::PlayerManager(QObject *parent) : QObject(parent) {}

bool PlayerManager::isPlaying() const {
    return m_isInPlayingState;
}

bool PlayerManager::canPlay() const {
    return m_currentTrack.isValid();
}

bool PlayerManager::canSkipBackward() const {
    return m_previousTrack.isValid();
}

bool PlayerManager::canSkipForward() const {
    return m_nextTrack.isValid();
}

void PlayerManager::playing() {
    if (!m_isInPlayingState) {
        const auto oldPreviousTrackIsEnabled = canSkipBackward();
        const auto oldNextTrackIsEnabled = canSkipForward();

        m_isInPlayingState = true;
        Q_EMIT isPlayingChanged();

        if (!m_currentTrack.isValid()) return;

        if (oldNextTrackIsEnabled != canSkipForward()) {
            Q_EMIT canSkipForwardChanged();
        }

        if (oldPreviousTrackIsEnabled != canSkipBackward()) {
            Q_EMIT canSkipBackwardChanged();
        }
    }
}

void PlayerManager::pausedOrStopped() {
    if (m_isInPlayingState) {
        const auto oldPreviousTrackIsEnabled = canSkipBackward();
        const auto oldNextTrackIsEnabled = canSkipForward();

        m_isInPlayingState = false;
        Q_EMIT isPlayingChanged();

        if (!m_currentTrack.isValid()) return;

        if (oldNextTrackIsEnabled != canSkipForward()) {
            Q_EMIT canSkipForwardChanged();
        }

        if (oldPreviousTrackIsEnabled != canSkipBackward()) {
            Q_EMIT canSkipBackwardChanged();
        }
    }
}

void PlayerManager::setPreviousTrack(const QPersistentModelIndex &previousTrack) {
    if (m_previousTrack == previousTrack) return;

    const bool oldCanSkipBackward = canSkipBackward();

    m_previousTrack = previousTrack;
    Q_EMIT previousTrackChanged();

    if (oldCanSkipBackward != canSkipBackward()) {
        Q_EMIT canSkipBackwardChanged();
    }
}

void PlayerManager::setCurrentTrack(const QPersistentModelIndex &currentTrack) {
    if (m_currentTrack == currentTrack) return;

    const bool oldCanPlay = canPlay();

    m_currentTrack = currentTrack;
    Q_EMIT currentTrackChanged();

    if (oldCanPlay != canPlay()) {
        Q_EMIT canPlayChanged();
    }
}

void PlayerManager::setNextTrack(const QPersistentModelIndex &nextTrack) {
    if (m_nextTrack == nextTrack) return;

    const bool oldCanSkipForward = canSkipForward();

    m_nextTrack = nextTrack;
    Q_EMIT nextTrackChanged();

    if (oldCanSkipForward != canSkipForward()) {
        Q_EMIT canSkipForwardChanged();
    }
}

QPersistentModelIndex PlayerManager::currentTrack() const {
    return m_currentTrack;
}

QPersistentModelIndex PlayerManager::previousTrack() const {
    return m_previousTrack;
}

QPersistentModelIndex PlayerManager::nextTrack() const {
    return m_nextTrack;
}
