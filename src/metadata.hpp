#ifndef METADATA_H
#define METADATA_H

#include "playerutils.hpp"

#include <QList>
#include <QMap>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>
#include <QVariant>

#include <utility>

class Metadata : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum Fields : std::uint16_t {
        Title = Qt::UserRole + 1,
        Artist,
        Album,
        AlbumArtist,
        DatabaseId,
        ResourceUrl,
        Genre,
        Composer,
        Lyricist,
        Conductor,
        Performer,
        Year,
        TrackNumber,
        DiscNumber,
        Duration,
        DurationString,
        BitRate,
        SampleRate,
        Channels,
        CoverImage,
        HasEmbeddedCover,
        AlbumId,
        Comment,
        Lyrics,
        Rating,
        LastPlayed,
        DateAdded,
        DateModified,
        FileType,
        ElementType,
        Hash,
        IsValid
    };

    Q_ENUM(Metadata::Fields)

    struct TrackFields {
        QHash<Fields, QVariant> data;

        void insert(Fields field, const QVariant& value);
        [[nodiscard]] bool isValid() const;
        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] QVariant get(Fields field) const;
        [[nodiscard]] bool contains(Fields field) const;
        [[nodiscard]] QString generateHash() const;
    };

    // TODO: is this necessary
    struct EntryFields {
        TrackFields trackFields;
        QString title;
        QUrl url;

        [[nodiscard]] bool isValid() const;
    };

    using EntryFieldsList = QList<EntryFields>;

    enum PlaylistFields : std::uint16_t {
        IsPlaying = static_cast<int>(Fields::IsValid) + 1,
        AlbumSection,
        MetadataModifiable
    };
};

//     using MetadataField = QMap<ColumnsRoles, QVariant>;
//
//     class MusicMetadataField : public MetadataField {
//     public:
//         using MetadataField::MetadataField;
//
//         [[nodiscard]] bool hasDatabaseId() const {
//             return contains(DatabaseIdRole);
//         }
//
//         [[nodiscard]] qulonglong databaseId() const {
//             return value(DatabaseIdRole).toULongLong();
//         }
//
//         [[nodiscard]] bool hasElementType() const {
//             return contains(ElementTypeRole);
//         }
//
//         [[nodiscard]] PlayerUtils::PlaylistEntryType elementType() const {
//             return value(ElementTypeRole).value<PlayerUtils::PlaylistEntryType>();
//         }
//     };
//
//     class TrackMetadataField : public MusicMetadataField {
//     public:
//         using MusicMetadataField::MusicMetadataField;
//
//         TrackMetadataField(bool aValid, QString aId, QString aParentId, QString aTitle, QString aArtist,
//                            QString aAlbumName, QString aAlbumArtist, int aTrackNumber, int aDiscNumber, QTime
//                            aDuration, QUrl aResourceURI, QString aFileType, const QDateTime &fileModificationTime,
//                            QUrl aAlbumCover, bool aIsSingleDiscAlbum, QString aGenre, QString aPerformer,
//                            QString aComposer, QString aLyricist, bool aHasEmbeddedCover, QString aHash)
//             : MusicMetadataField({
//                   {key_type::TitleRole, std::move(aTitle)},
//                   {key_type::AlbumRole, std::move(aAlbumName)},
//                   {key_type::ArtistRole, std::move(aArtist)},
//                   {key_type::AlbumArtistRole, std::move(aAlbumArtist)},
//                   {key_type::IdRole, std::move(aId)},
//                   {key_type::ParentIdRole, std::move(aParentId)},
//                   {key_type::TrackNumberRole, aTrackNumber},
//                   {key_type::DiscNumberRole, aDiscNumber},
//                   {key_type::DurationRole, aDuration},
//                   {key_type::ResourceRole, std::move(aResourceURI)},
//                   {key_type::FileTypeRole, std::move(aFileType)},
//                   {key_type::FileModificationTime, fileModificationTime},
//                   {key_type::ImageUrlRole, std::move(aAlbumCover)},
//                   {key_type::IsSingleDiscAlbumRole, aIsSingleDiscAlbum},
//                   {key_type::GenreRole, std::move(aGenre)},
//                   {key_type::PerformerRole, std::move(aPerformer)},
//                   {key_type::ComposerRole, std::move(aComposer)},
//                   {key_type::LyricistRole, std::move(aLyricist)},
//                   {key_type::HasEmbeddedCover, aHasEmbeddedCover},
//                   {key_type::HashRole, std::move(aHash)},
//               }) {
//             Q_UNUSED(aValid)
//         }
//
//         [[nodiscard]] bool isValid() const {
//             return !isEmpty() && duration().isValid();
//         }
//
//         [[nodiscard]] QString title() const {
//             return value(TitleRole).toString();
//         }
//
//         [[nodiscard]] bool hasRole(const ColumnsRoles role) const {
//             return contains(role);
//         }
//
//         [[nodiscard]] QString artist() const {
//             return value(ArtistRole).toString();
//         }
//
//         [[nodiscard]] QString album() const {
//             return value(AlbumRole).toString();
//         }
//
//         [[nodiscard]] QString albumArtist() const {
//             return value(AlbumArtistRole).toString();
//         }
//
//         [[nodiscard]] int trackNumber() const {
//             return value(TrackNumberRole).toInt();
//         }
//
//         [[nodiscard]] int discNumber() const {
//             return value(DiscNumberRole).toInt();
//         }
//
//         [[nodiscard]] QTime duration() const {
//             return value(DurationRole).toTime();
//         }
//
//         [[nodiscard]] QUrl resourceURI() const {
//             return value(ResourceRole).toUrl();
//         }
//
//         [[nodiscard]] QString fileType() const {
//             return value(FileTypeRole).toString();
//         }
//
//         [[nodiscard]] QUrl albumCover() const {
//             return value(ImageUrlRole).toUrl();
//         }
//
//         [[nodiscard]] bool isSingleDiscAlbum() const {
//             return value(IsSingleDiscAlbumRole).toBool();
//         }
//
//         [[nodiscard]] QString genre() const {
//             return value(GenreRole).toString();
//         }
//
//         [[nodiscard]] QString performer() const {
//             return value(PerformerRole).toString();
//         }
//
//         [[nodiscard]] QString composer() const {
//             return value(ComposerRole).toString();
//         }
//
//         [[nodiscard]] QString lyricist() const {
//             return value(LyricistRole).toString();
//         }
//
//         [[nodiscard]] QString lyrics() const {
//             return value(LyricsRole).toString();
//         }
//
//         [[nodiscard]] QString comment() const {
//             return value(CommentRole).toString();
//         }
//
//         [[nodiscard]] int year() const {
//             return value(YearRole).toInt();
//         }
//
//         [[nodiscard]] int channels() const {
//             return value(ChannelsRole).toInt();
//         }
//
//         [[nodiscard]] int bitRate() const {
//             return value(BitRateRole).toInt();
//         }
//
//         [[nodiscard]] int sampleRate() const {
//             return value(SampleRateRole).toInt();
//         }
//
//         [[nodiscard]] bool hasEmbeddedCover() const {
//             return value(HasEmbeddedCover).toBool();
//         }
//
//         [[nodiscard]] QString hash() const {
//             return value(HashRole).toString();
//         }
//
//         [[nodiscard]] QString generateHash() const {
//             return PlayerUtils::calculateTrackHash(title(), resourceURI().toString(), fileType(), artist(), album(),
//                                                    albumArtist(), genre());
//         }
//
//         [[nodiscard]] QDateTime fileModificationTime() const {
//             return value(FileModificationTime).toDateTime();
//         }
//
//         [[nodiscard]] bool albumInfoIsSame(const TrackMetadataField &other) const;
//
//         [[nodiscard]] bool isSameTrack(const TrackMetadataField &other) const;
//     };
//
//     using ListTrackMetadataField = QList<TrackMetadataField>;
//
//     class AlbumMetadataField : public MusicMetadataField {
//     public:
//         using MusicMetadataField::MusicMetadataField;
//
//         [[nodiscard]] QString title() const {
//             return value(TitleRole).toString();
//         }
//
//         [[nodiscard]] QString artist() const {
//             return value(ArtistRole).toString();
//         }
//
//         [[nodiscard]] bool isValidArtist() const {
//             const auto &artistData = value(ArtistRole);
//             return artistData.isValid() && !artistData.toString().isEmpty();
//         }
//
//         [[nodiscard]] QStringList genres() const {
//             return value(GenreRole).toStringList();
//         }
//
//         [[nodiscard]] QUrl albumArtURI() const {
//             return value(ImageUrlRole).toUrl();
//         }
//
//         [[nodiscard]] bool isSingleDiscAlbum() const {
//             return value(IsSingleDiscAlbumRole).toBool();
//         }
//
//         [[nodiscard]] bool isValid() const {
//             return !isEmpty();
//         }
//     };
//
//     using ListAlbumMetadataField = QList<AlbumMetadataField>;
//
//     class ArtistMetadataField : public MusicMetadataField {
//     public:
//         using MusicMetadataField::MusicMetadataField;
//
//         [[nodiscard]] QString name() const {
//             return value(TitleRole).toString();
//         }
//
//         [[nodiscard]] qulonglong databaseId() const {
//             return value(DatabaseIdRole).toULongLong();
//         }
//
//         [[nodiscard]] QUrl artistArtURI() const {
//             return value(ImageUrlRole).toUrl();
//         }
//     };
//
//     using ListArtistMetadataField = QList<ArtistMetadataField>;
//
//     class GenreMetadataField : public MusicMetadataField {
//     public:
//         using MusicMetadataField::MusicMetadataField;
//
//         [[nodiscard]] QString title() const {
//             return value(TitleRole).toString();
//         }
//     };
//
//     using ListGenreMetadataField = QList<GenreMetadataField>;
//
//     class PlaylistMetadataField : public MusicMetadataField {
//     public:
//         using MusicMetadataField::MusicMetadataField;
//
//         [[nodiscard]] QString name() const {
//             return value(TitleRole).toString();
//         }
//
//         [[nodiscard]] QUrl resourceURI() const {
//             return value(ResourceRole).toUrl();
//         }
//
//         [[nodiscard]] qulonglong databaseId() const {
//             return value(DatabaseIdRole).toULongLong();
//         }
//     };
//
//     using ListPlaylistMetadataField = QList<PlaylistMetadataField>;
//
//     struct EntryMetadata {
//         MusicMetadataField musicMetadata;
//         QString title;
//         QUrl url;
//
//         [[nodiscard]] bool isValid() const {
//             return !musicMetadata.isEmpty() || !title.isEmpty() || !url.isEmpty();
//         }
//     };
//
//     using EntryMetadataList = QList<EntryMetadata>;
// };

Q_DECLARE_METATYPE(Metadata::TrackFields)
Q_DECLARE_METATYPE(Metadata::EntryFields)
Q_DECLARE_METATYPE(Metadata::EntryFieldsList)
Q_DECLARE_METATYPE(Metadata::PlaylistFields)

// Q_DECLARE_METATYPE(MetadataFields::MusicMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::TrackMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::AlbumMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::ArtistMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::GenreMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::PlaylistMetadataField)
//
// Q_DECLARE_METATYPE(MetadataFields::ListTrackMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::ListAlbumMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::ListArtistMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::ListGenreMetadataField)
// Q_DECLARE_METATYPE(MetadataFields::ListPlaylistMetadataField)
//
// Q_DECLARE_METATYPE(MetadataFields::EntryMetadata)
// Q_DECLARE_METATYPE(MetadataFields::EntryMetadataList)

#endif // METADATA_H
