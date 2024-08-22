#include "trackmetadatamanager.h"

TrackMetadataManager::TrackMetadataManager(QObject *parent) : QObject(parent) {}

QPersistentModelIndex TrackMetadataManager::currentTrack() const {
    return m_currentTrack;
}

int TrackMetadataManager::artistRole() const {
    return m_artistRole;
}

int TrackMetadataManager::titleRole() const {
    return m_titleRole;
}

int TrackMetadataManager::albumRole() const {
    return m_albumRole;
}

int TrackMetadataManager::albumArtistRole() const {
    return m_albumArtistRole;
}

int TrackMetadataManager::fileNameRole() const {
    return m_fileNameRole;
}

int TrackMetadataManager::imageRole() const {
    return m_imageRole;
}

int TrackMetadataManager::databaseIdRole() const {
    return m_databaseIdRole;
}

int TrackMetadataManager::trackTypeRole() const {
    return m_trackTypeRole;
}

int TrackMetadataManager::albumIdRole() const {
    return m_albumIdRole;
}

int TrackMetadataManager::isValidRole() const {
    return m_isValidRole;
}

QVariant TrackMetadataManager::artist() const {
    if (!m_currentTrack.isValid()) {
        return QString();
    }

    auto artistData = m_currentTrack.data(m_artistRole);

    return artistData.isValid() ? artistData : m_currentTrack.data(m_albumArtistRole);
}

QVariant TrackMetadataManager::title() const {
    return m_currentTrack.isValid() ? m_currentTrack.data(m_titleRole) : QString();
}

QVariant TrackMetadataManager::album() const {
    return m_currentTrack.isValid() && !m_currentTrack.data(m_albumRole).isNull()
           ? m_currentTrack.data(m_albumRole)
           : QVariant{QString{}};
}

QVariant TrackMetadataManager::albumArtist() const {
    return m_currentTrack.isValid() ? m_currentTrack.data(m_albumArtistRole) : QString();
}

QUrl TrackMetadataManager::fileUrl() const {
    return m_currentTrack.isValid() ? m_currentTrack.data(m_fileNameRole).toUrl() : QUrl{};
}

QUrl TrackMetadataManager::image() const {
    return m_currentTrack.isValid() ? m_currentTrack.data(m_imageRole).toUrl() : QUrl{};
}

qulonglong TrackMetadataManager::databaseId() const {
    return m_currentTrack.isValid() ? m_currentTrack.data(m_databaseIdRole).toULongLong() : 0;
}

PlayerUtils::PlaylistEntryType TrackMetadataManager::trackType() const {
    return m_currentTrack.isValid()
        ? m_currentTrack.data(m_trackTypeRole).value<PlayerUtils::PlaylistEntryType>()
        : PlayerUtils::Unknown;
}

qulonglong TrackMetadataManager::albumId() const {
    return m_currentTrack.isValid() ? m_currentTrack.data(m_albumIdRole).toULongLong() : 0;
}

bool TrackMetadataManager::isValid() const {
    return m_currentTrack.isValid() && m_currentTrack.data(m_isValidRole).toBool();
}

void TrackMetadataManager::setCurrentTrack(const QPersistentModelIndex &currentTrack) {
    m_currentTrack = currentTrack;
    Q_EMIT currentTrackChanged();
    updateCurrentTrackMetadata();
}

void TrackMetadataManager::setArtistRole(int newArtistRole) {
    m_artistRole = newArtistRole;
    Q_EMIT artistRoleChanged();
}

void TrackMetadataManager::setTitleRole(int newTitleRole) {
    m_titleRole = newTitleRole;
    Q_EMIT titleRoleChanged();
}

void TrackMetadataManager::setAlbumRole(int newAlbumRole) {
    m_albumRole = newAlbumRole;
    Q_EMIT albumRoleChanged();
}

void TrackMetadataManager::setAlbumArtistRole(int newAlbumArtistRole) {
    m_albumArtistRole = newAlbumArtistRole;
    Q_EMIT albumArtistRoleChanged();
}

void TrackMetadataManager::setFileNameRole(int newFileNameRole) {
    m_fileNameRole = newFileNameRole;
    Q_EMIT fileNameRoleChanged();
}

void TrackMetadataManager::setImageRole(int newImageRole) {
    m_imageRole = newImageRole;
    Q_EMIT imageRoleChanged();
}

void TrackMetadataManager::setDatabaseIdRole(int newDatabaseIdRole) {
    m_databaseIdRole = newDatabaseIdRole;
    Q_EMIT databaseIdRoleChanged();
}

void TrackMetadataManager::setTrackTypeRole(int newTrackTypeRole) {
    m_trackTypeRole = newTrackTypeRole;
    Q_EMIT trackTypeRoleChanged();
}

void TrackMetadataManager::setAlbumIdRole(int newAlbumIdRole) {
    m_albumIdRole = newAlbumIdRole;
    Q_EMIT albumIdRoleChanged();
}

void TrackMetadataManager::setIsValidRole(int newIsValidRole) {
    m_isValidRole = newIsValidRole;
    Q_EMIT isValidRoleChanged();
}

void TrackMetadataManager::notifyArtistProperty() {
    auto newArtistValue = m_currentTrack.data(m_artistRole);

    if (m_oldArtist != newArtistValue) {
        Q_EMIT artistChanged();
        m_oldArtist = newArtistValue;
    }
}

void TrackMetadataManager::notifyTitleProperty() {
    auto newTitleValue = m_currentTrack.data(m_titleRole);

    if (m_oldTitle != newTitleValue) {
        Q_EMIT titleChanged();
        m_oldTitle = newTitleValue;
    }
}

void TrackMetadataManager::notifyAlbumProperty() {
    auto newAlbumValue = m_currentTrack.data(m_albumRole);

    if (m_oldAlbum != newAlbumValue) {
        Q_EMIT albumChanged();
        m_oldAlbum = newAlbumValue;
    }
}

void TrackMetadataManager::notifyAlbumArtistProperty() {
    auto newAlbumArtistValue = m_currentTrack.data(m_albumArtistRole);

    if (m_oldAlbumArtist != newAlbumArtistValue) {
        Q_EMIT albumArtistChanged();
        m_oldAlbumArtist = newAlbumArtistValue;
    }
}

void TrackMetadataManager::notifyFileNameProperty() {
    auto newFileNameValue = m_currentTrack.data(m_fileNameRole);

    if (m_oldFileName != newFileNameValue) {
        Q_EMIT fileUrlChanged();
        m_oldFileName = newFileNameValue;
    }
}

void TrackMetadataManager::notifyImageProperty() {
    auto newImageValue = m_currentTrack.data(m_imageRole);

    if (m_oldImage != newImageValue) {
        Q_EMIT imageChanged();
        m_oldImage = newImageValue;
    }
}

void TrackMetadataManager::notifyDatabaseIdProperty() {
    bool conversionOk;

    auto newDatabaseIdValue = m_currentTrack.data(m_databaseIdRole).toULongLong(&conversionOk);

    if (conversionOk && m_oldDatabaseId != newDatabaseIdValue) {
        Q_EMIT databaseIdChanged();
        m_oldDatabaseId = newDatabaseIdValue;
    } else if (!conversionOk && m_oldDatabaseId != 0) {
        Q_EMIT databaseIdChanged();
        m_oldDatabaseId = 0;
    }
}

void TrackMetadataManager::notifyTrackTypeProperty() {
    auto newTrackTypeValue = m_currentTrack.data(m_trackTypeRole).value<PlayerUtils::PlaylistEntryType>();

    if (m_oldTrackType != newTrackTypeValue) {
        Q_EMIT trackTypeChanged();
        m_oldTrackType = newTrackTypeValue;
    }
}

void TrackMetadataManager::notifyAlbumIdProperty() {
    bool conversionOk;

    auto newAlbumIdValue = m_currentTrack.data(m_albumIdRole).toULongLong(&conversionOk);

    if (conversionOk && m_oldAlbumId != newAlbumIdValue) {
        Q_EMIT albumIdChanged();
        m_oldAlbumId = newAlbumIdValue;
    } else if (!conversionOk && m_oldAlbumId != 0) {
        Q_EMIT albumIdChanged();
        m_oldAlbumId = 0;
    }
}

void TrackMetadataManager::notifyIsValidProperty() {
    auto newIsValidValue = m_currentTrack.data(m_isValidRole).toBool();

    if (m_oldIsValid != newIsValidValue) {
        Q_EMIT isValidChanged();
        m_oldIsValid = newIsValidValue;
    }
}

void TrackMetadataManager::updateCurrentTrackMetadata() {
    notifyArtistProperty();
    notifyTitleProperty();
    notifyAlbumProperty();
    notifyAlbumArtistProperty();
    notifyFileNameProperty();
    notifyImageProperty();
    notifyDatabaseIdProperty();
    notifyTrackTypeProperty();
    notifyAlbumIdProperty();
    notifyIsValidProperty();
}
