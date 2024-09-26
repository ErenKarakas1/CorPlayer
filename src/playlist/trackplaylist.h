#ifndef TRACKPLAYLIST_H
#define TRACKPLAYLIST_H

#include "metadatafields.h"
#include "playerutils.h"

#include <QAbstractListModel>
#include <QMediaPlayer>
#include <QQmlEngine>

#include <memory>
#include <utility>

class TrackPlaylistPrivate;
class TrackPlaylistEntry;
class QDebug;

class TrackPlaylist : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum ColumnRoles {
        TitleRole = MetadataFields::TitleRole,
        SecondaryTextRole,
        ImageUrlRole,
        ShadowForImageRole,
        ChildModelRole,
        DurationRole,
        StringDurationRole,
        ArtistRole,
        AllArtistsRole,
        AlbumRole,
        AlbumArtistRole,
        IsValidAlbumArtistRole,
        TrackNumberRole,
        DiscNumberRole,
        GenreRole,
        PerformerRole,
        LyricistRole,
        ComposerRole,
        CommentRole,
        YearRole,
        ChannelsRole,
        BitRateRole,
        SampleRateRole,
        ResourceRole,
        FileTypeRole,
        IdRole,
        ParentIdRole,
        DatabaseIdRole,
        IsSingleDiscAlbumRole,
        ContainerDataRole,
        IsPartialDataRole,
        AlbumIdRole,
        HasEmbeddedCover,
        FileModificationTime,
        ElementTypeRole,
        LyricsRole,
        FullDataRole,
        IsDirectoryRole,
        IsPlaylistRole,
        FilePathRole,
        IsValidRole,
        CountRole,
        IsPlayingRole,
        AlbumSectionRole,
        MetadataModifiableRole,
        HashRole,
    };

    Q_ENUM(ColumnRoles)

    enum PlayState {
        NotPlaying,
        IsPlaying,
        IsPaused,
    };

    Q_ENUM(PlayState)

    using ListTrackMetadataField = MetadataFields::ListTrackMetadataField;
    using TrackMetadataField = MetadataFields::TrackMetadataField;

    explicit TrackPlaylist(QObject *parent = nullptr);
    ~TrackPlaylist() override;

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent,
                  int destinationChild) override;

    void clearPlaylist();
    void enqueueRestoredEntries(const QVariantList &newEntries);
    [[nodiscard]] QVariantList getEntriesForRestore() const;

Q_SIGNALS:
    void addTrackByName(const QVariant &title, const QVariant &artist, const QVariant &album,
                        const QVariant &trackNumber, const QVariant &discNumber);

    void addNewEntry(qulonglong newDatabaseId, const QString &entryTitle,
                     PlayerUtils::PlaylistEntryType databaseIdType);

    void addNewUrl(const QUrl &entryUrl, PlayerUtils::PlaylistEntryType databaseIdType);

public Q_SLOTS:
    void tracksListAdded(qulonglong newDatabaseId, const QString &entryTitle,
                         PlayerUtils::PlaylistEntryType databaseIdType,
                         const TrackPlaylist::ListTrackMetadataField &tracks);

    void trackChanged(const TrackPlaylist::TrackMetadataField &track);
    void trackRemoved(qulonglong trackId);
    void trackInError(const QUrl &sourceInError, QMediaPlayer::Error playerError);

    /**
     * If `insertAt` is negative or larger than the playlist size, the entries
     * will be enqueued at the end of the playlist.
     */
    void enqueueOneEntry(const MetadataFields::EntryMetadata &entryData, int insertAt = -1);
    void enqueueMultipleEntries(const MetadataFields::EntryMetadataList &entriesData, int insertAt = -1);

private:
    std::unique_ptr<TrackPlaylistPrivate> tp;
};

class TrackPlaylistEntry {
public:
    TrackPlaylistEntry() = default;

    TrackPlaylistEntry(qulonglong id, QVariant title, QVariant artist, QVariant album, QVariant trackUrl,
                       QVariant trackNumber, QVariant discNumber,
                       PlayerUtils::PlaylistEntryType entryType = PlayerUtils::Unknown)
        : m_title(std::move(title)), m_album(std::move(album)), m_artist(std::move(artist)),
          m_trackUrl(std::move(trackUrl)), m_trackNumber(std::move(trackNumber)), m_discNumber(std::move(discNumber)),
          m_id(id), m_entryType(entryType) {}

    explicit TrackPlaylistEntry(const TrackPlaylist::TrackMetadataField &track)
        : m_title(track[MetadataFields::TitleRole]), m_album(track[MetadataFields::AlbumRole]),
          m_trackNumber(track[MetadataFields::TrackNumberRole]), m_discNumber(track[MetadataFields::DiscNumberRole]),
          m_id(track[MetadataFields::DatabaseIdRole].toULongLong()), m_isValid(true) {}

    explicit TrackPlaylistEntry(const QUrl &fileName) : m_trackUrl(fileName) {}

    explicit TrackPlaylistEntry(const qulonglong id, const QString &entryTitle,
                                const PlayerUtils::PlaylistEntryType type)
        : m_title(entryTitle), m_id(id), m_isValid(true), m_entryType(type) {}

    QVariant m_title;
    QVariant m_album;
    QVariant m_artist;
    QVariant m_trackUrl;
    QVariant m_trackNumber;
    QVariant m_discNumber;
    qulonglong m_id = 0;
    bool m_isValid = false;
    PlayerUtils::PlaylistEntryType m_entryType = PlayerUtils::Unknown;
    TrackPlaylist::PlayState m_isPlaying = TrackPlaylist::NotPlaying;
};

QDebug operator<<(const QDebug &stream, const TrackPlaylistEntry &data);

#endif // TRACKPLAYLIST_H
