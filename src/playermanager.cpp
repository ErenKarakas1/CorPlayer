#include "playermanager.h"

PlayerManager::PlayerManager(QObject *parent) : QObject(parent) {}

bool PlayerManager::playControlEnabled() const {
    return m_currentTrack.isValid();
}

bool PlayerManager::skipBackwardControlEnabled() const {
    return m_previousTrack.isValid();
}

bool PlayerManager::skipForwardControlEnabled() const {
    return m_nextTrack.isValid();
}

bool PlayerManager::musicPlaying() const {
    return m_isInPlayingState;
}

void PlayerManager::playerPausedOrStopped() {
    if (m_isInPlayingState) {
        const auto oldPreviousTrackIsEnabled = skipBackwardControlEnabled();
        const auto oldNextTrackIsEnabled = skipForwardControlEnabled();

        m_isInPlayingState = false;
        Q_EMIT musicPlayingChanged();

        if (!m_currentTrack.isValid()) return;

        if (oldNextTrackIsEnabled != skipForwardControlEnabled()) {
            Q_EMIT skipForwardControlEnabledChanged();
        }

        if (oldPreviousTrackIsEnabled != skipBackwardControlEnabled()) {
            Q_EMIT skipBackwardControlEnabledChanged();
        }
    }
}

void PlayerManager::playerPlaying() {
    if (!m_isInPlayingState) {
        const auto oldPreviousTrackIsEnabled = skipBackwardControlEnabled();
        const auto oldNextTrackIsEnabled = skipForwardControlEnabled();

        m_isInPlayingState = true;
        Q_EMIT musicPlayingChanged();

        if (!m_currentTrack.isValid()) return;

        if (oldNextTrackIsEnabled != skipForwardControlEnabled()) {
            Q_EMIT skipForwardControlEnabledChanged();
        }

        if (oldPreviousTrackIsEnabled != skipBackwardControlEnabled()) {
            Q_EMIT skipBackwardControlEnabledChanged();
        }
    }
}

void PlayerManager::setPreviousTrack(const QPersistentModelIndex &previousTrack) {
    if (m_previousTrack == previousTrack) return;

    const bool oldValueSkipBackward = skipBackwardControlEnabled();

    m_previousTrack = previousTrack;
    Q_EMIT previousTrackChanged();

    if (oldValueSkipBackward != skipBackwardControlEnabled()) {
        Q_EMIT skipBackwardControlEnabledChanged();
    }
}

void PlayerManager::setCurrentTrack(const QPersistentModelIndex &currentTrack) {
    if (m_currentTrack == currentTrack) return;

    const bool oldPlayControlEnabled = playControlEnabled();

    m_currentTrack = currentTrack;
    Q_EMIT currentTrackChanged();

    if (oldPlayControlEnabled != playControlEnabled()) {
        Q_EMIT playControlEnabledChanged();
    }
}

void PlayerManager::setNextTrack(const QPersistentModelIndex &nextTrack) {
    if (m_nextTrack == nextTrack) return;

    const bool oldValueSkipForward = skipForwardControlEnabled();

    m_nextTrack = nextTrack;
    Q_EMIT nextTrackChanged();

    if (oldValueSkipForward != skipForwardControlEnabled()) {
        Q_EMIT skipForwardControlEnabledChanged();
    }
}

QPersistentModelIndex PlayerManager::previousTrack() const {
    return m_previousTrack;
}

QPersistentModelIndex PlayerManager::currentTrack() const {
    return m_currentTrack;
}

QPersistentModelIndex PlayerManager::nextTrack() const {
    return m_nextTrack;
}
