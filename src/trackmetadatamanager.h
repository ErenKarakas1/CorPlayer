#ifndef TRACKMETADATAMANAGER_H
#define TRACKMETADATAMANAGER_H

#include <QUrl>
#include <QObject>
#include <QQmlEngine>
#include <QPersistentModelIndex>

#include "playerutils.h"

class TrackMetadataManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QPersistentModelIndex currentTrack
        READ currentTrack
        WRITE setCurrentTrack
        NOTIFY currentTrackChanged)

    Q_PROPERTY(int artistRole READ artistRole WRITE setArtistRole NOTIFY artistRoleChanged)
    Q_PROPERTY(int titleRole READ titleRole WRITE setTitleRole NOTIFY titleRoleChanged)
    Q_PROPERTY(int albumRole READ albumRole WRITE setAlbumRole NOTIFY albumRoleChanged)
    Q_PROPERTY(int albumArtistRole READ albumArtistRole WRITE setAlbumArtistRole NOTIFY albumArtistRoleChanged)
    Q_PROPERTY(int fileNameRole READ fileNameRole WRITE setFileNameRole NOTIFY fileNameRoleChanged)
    Q_PROPERTY(int imageRole READ imageRole WRITE setImageRole NOTIFY imageRoleChanged)
    Q_PROPERTY(int databaseIdRole READ databaseIdRole WRITE setDatabaseIdRole NOTIFY databaseIdRoleChanged)
    Q_PROPERTY(int trackTypeRole READ trackTypeRole WRITE setTrackTypeRole NOTIFY trackTypeRoleChanged)
    Q_PROPERTY(int albumIdRole READ albumIdRole WRITE setAlbumIdRole NOTIFY albumIdRoleChanged)
    Q_PROPERTY(int isValidRole READ isValidRole WRITE setIsValidRole NOTIFY isValidRoleChanged)

    Q_PROPERTY(QVariant artist READ artist NOTIFY artistChanged)
    Q_PROPERTY(QVariant title READ title NOTIFY titleChanged)
    Q_PROPERTY(QVariant album READ album NOTIFY albumChanged)
    Q_PROPERTY(QVariant albumArtist READ albumArtist NOTIFY albumArtistChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl image READ image NOTIFY imageChanged)
    Q_PROPERTY(qulonglong databaseId READ databaseId NOTIFY databaseIdChanged)
    Q_PROPERTY(PlayerUtils::PlaylistEntryType trackType READ trackType NOTIFY trackTypeChanged)
    Q_PROPERTY(qulonglong albumId READ albumId NOTIFY albumIdChanged)
    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)

public:
    explicit TrackMetadataManager(QObject *parent = nullptr);
    [[nodiscard]] QPersistentModelIndex currentTrack() const;

    [[nodiscard]] int artistRole() const;
    [[nodiscard]] int titleRole() const;
    [[nodiscard]] int albumRole() const;
    [[nodiscard]] int albumArtistRole() const;
    [[nodiscard]] int fileNameRole() const;
    [[nodiscard]] int imageRole() const;
    [[nodiscard]] int databaseIdRole() const;
    [[nodiscard]] int trackTypeRole() const;
    [[nodiscard]] int albumIdRole() const;
    [[nodiscard]] int isValidRole() const;

    [[nodiscard]] QVariant artist() const;
    [[nodiscard]] QVariant title() const;
    [[nodiscard]] QVariant album() const;
    [[nodiscard]] QVariant albumArtist() const;
    [[nodiscard]] QUrl fileUrl() const;
    [[nodiscard]] QUrl image() const;
    [[nodiscard]] qulonglong databaseId() const;
    [[nodiscard]] PlayerUtils::PlaylistEntryType trackType() const;
    [[nodiscard]] qulonglong albumId() const;
    [[nodiscard]] bool isValid() const;

Q_SIGNALS:
    void currentTrackChanged();
    void artistRoleChanged();
    void titleRoleChanged();
    void albumRoleChanged();
    void albumArtistRoleChanged();
    void fileNameRoleChanged();
    void imageRoleChanged();
    void databaseIdRoleChanged();
    void trackTypeRoleChanged();
    void albumIdRoleChanged();
    void isValidRoleChanged();

    void artistChanged();
    void titleChanged();
    void albumChanged();
    void albumArtistChanged();
    void fileUrlChanged();
    void imageChanged();
    void databaseIdChanged();
    void trackTypeChanged();
    void albumIdChanged();
    void isValidChanged();

public Q_SLOTS:
    void setCurrentTrack(const QPersistentModelIndex &currentTrack);
    void updateCurrentTrackMetadata();
    void setArtistRole(int newArtistRole);
    void setTitleRole(int newTitleRole);
    void setAlbumRole(int newAlbumRole);
    void setAlbumArtistRole(int newAlbumArtistRole);
    void setFileNameRole(int newFileNameRole);
    void setImageRole(int newImageRole);
    void setDatabaseIdRole(int newDatabaseIdRole);
    void setTrackTypeRole(int newTrackTypeRole);
    void setAlbumIdRole(int newAlbumIdRole);
    void setIsValidRole(int newIsValidRole);

private:
    void notifyArtistProperty();
    void notifyTitleProperty();
    void notifyAlbumProperty();
    void notifyAlbumArtistProperty();
    void notifyFileNameProperty();
    void notifyImageProperty();
    void notifyDatabaseIdProperty();
    void notifyTrackTypeProperty();
    void notifyAlbumIdProperty();
    void notifyIsValidProperty();

    QPersistentModelIndex m_currentTrack;
    int m_artistRole = Qt::DisplayRole;
    int m_titleRole = Qt::DisplayRole;
    int m_albumRole = Qt::DisplayRole;
    int m_albumArtistRole = Qt::DisplayRole;
    int m_fileNameRole = Qt::DisplayRole;
    int m_imageRole = Qt::DisplayRole;
    int m_databaseIdRole = Qt::DisplayRole;
    int m_trackTypeRole = Qt::DisplayRole;
    int m_albumIdRole = Qt::DisplayRole;
    int m_isValidRole = Qt::DisplayRole;

    QVariant m_oldArtist;
    QVariant m_oldTitle;
    QVariant m_oldAlbum;
    QVariant m_oldAlbumArtist;
    QVariant m_oldFileName;
    QVariant m_oldImage;
    qulonglong m_oldDatabaseId = 0;
    PlayerUtils::PlaylistEntryType m_oldTrackType = PlayerUtils::Unknown;
    qulonglong m_oldAlbumId = 0;
    bool m_oldIsValid = false;
};

#endif //TRACKMETADATAMANAGER_H
