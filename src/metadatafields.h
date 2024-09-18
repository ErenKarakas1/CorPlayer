#ifndef METADATAFIELDS_H
#define METADATAFIELDS_H

#include "playerutils.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>
#include <QVariant>

#include <utility>

class MetadataFields : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum ColumnsRoles {
        TitleRole = Qt::UserRole + 1,
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
        LyricistRole,
        ComposerRole,
        CommentRole,
        YearRole,
        ChannelsRole,
        BitRateRole,
        SampleRateRole,
        ResourceRole,
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
        HasChildrenRole,
        MultipleImageUrlsRole,
    };

    Q_ENUM(ColumnsRoles)

    using MetadataField = QMap<ColumnsRoles, QVariant>;

    class MusicMetadataField : public MetadataField {
    public:
        using MetadataField::MetadataField;

        [[nodiscard]] bool hasDatabaseId() const {
            return contains(DatabaseIdRole);
        }

        [[nodiscard]] qulonglong databaseId() const {
            return value(DatabaseIdRole).toULongLong();
        }

        [[nodiscard]] bool hasElementType() const {
            return contains(ElementTypeRole);
        }

        [[nodiscard]] PlayerUtils::PlaylistEntryType elementType() const {
            return value(ElementTypeRole).value<PlayerUtils::PlaylistEntryType>();
        }
    };

    class TrackMetadataField : public MusicMetadataField {
    public:
        using MusicMetadataField::MusicMetadataField;

        TrackMetadataField(bool aValid, QString aId, QString aParentId, QString aTitle, QString aArtist,
                           QString aAlbumName, QString aAlbumArtist, int aTrackNumber, int aDiscNumber, QTime aDuration,
                           QUrl aResourceURI, const QDateTime &fileModificationTime, QUrl aAlbumCover,
                           bool aIsSingleDiscAlbum, QString aGenre, QString aComposer, QString aLyricist,
                           bool aHasEmbeddedCover)
            : MusicMetadataField({
                  {key_type::TitleRole, std::move(aTitle)},
                  {key_type::AlbumRole, std::move(aAlbumName)},
                  {key_type::ArtistRole, std::move(aArtist)},
                  {key_type::AlbumArtistRole, std::move(aAlbumArtist)},
                  {key_type::IdRole, std::move(aId)},
                  {key_type::ParentIdRole, std::move(aParentId)},
                  {key_type::TrackNumberRole, aTrackNumber},
                  {key_type::DiscNumberRole, aDiscNumber},
                  {key_type::DurationRole, aDuration},
                  {key_type::ResourceRole, std::move(aResourceURI)},
                  {key_type::FileModificationTime, fileModificationTime},
                  {key_type::ImageUrlRole, std::move(aAlbumCover)},
                  {key_type::IsSingleDiscAlbumRole, aIsSingleDiscAlbum},
                  {key_type::GenreRole, std::move(aGenre)},
                  {key_type::ComposerRole, std::move(aComposer)},
                  {key_type::LyricistRole, std::move(aLyricist)},
                  {key_type::HasEmbeddedCover, aHasEmbeddedCover},
              }) {
            Q_UNUSED(aValid)
        }

        [[nodiscard]] bool isValid() const {
            return !isEmpty() && duration().isValid();
        }

        [[nodiscard]] QString title() const {
            return value(TitleRole).toString();
        }

        [[nodiscard]] bool hasRole(ColumnsRoles role) const {
            return contains(role);
        }

        [[nodiscard]] QString artist() const {
            return value(ArtistRole).toString();
        }

        [[nodiscard]] QString album() const {
            return value(AlbumRole).toString();
        }

        [[nodiscard]] QString albumArtist() const {
            return value(AlbumArtistRole).toString();
        }

        [[nodiscard]] int trackNumber() const {
            return value(TrackNumberRole).toInt();
        }

        [[nodiscard]] int discNumber() const {
            return value(DiscNumberRole).toInt();
        }

        [[nodiscard]] QTime duration() const {
            return value(DurationRole).toTime();
        }

        [[nodiscard]] QUrl resourceURI() const {
            return value(ResourceRole).toUrl();
        }

        [[nodiscard]] QUrl albumCover() const {
            return value(ImageUrlRole).toUrl();
        }

        [[nodiscard]] bool isSingleDiscAlbum() const {
            return value(IsSingleDiscAlbumRole).toBool();
        }

        [[nodiscard]] QString genre() const {
            return value(GenreRole).toString();
        }

        [[nodiscard]] QString composer() const {
            return value(ComposerRole).toString();
        }

        [[nodiscard]] QString lyricist() const {
            return value(LyricistRole).toString();
        }

        [[nodiscard]] QString lyrics() const {
            return value(LyricsRole).toString();
        }

        [[nodiscard]] QString comment() const {
            return value(CommentRole).toString();
        }

        [[nodiscard]] int year() const {
            return value(YearRole).toInt();
        }

        [[nodiscard]] int channels() const {
            return value(ChannelsRole).toInt();
        }

        [[nodiscard]] int bitRate() const {
            return value(BitRateRole).toInt();
        }

        [[nodiscard]] int sampleRate() const {
            return value(SampleRateRole).toInt();
        }

        [[nodiscard]] bool hasEmbeddedCover() const {
            return value(HasEmbeddedCover).toBool();
        }

        [[nodiscard]] QDateTime fileModificationTime() const {
            return value(FileModificationTime).toDateTime();
        }

        [[nodiscard]] bool albumInfoIsSame(const TrackMetadataField &other) const;

        [[nodiscard]] bool isSameTrack(const TrackMetadataField &other) const;
    };

    using ListTrackMetadataField = QList<TrackMetadataField>;

    class AlbumMetadataField : public MusicMetadataField {
    public:
        using MusicMetadataField::MusicMetadataField;

        [[nodiscard]] QString title() const {
            return value(TitleRole).toString();
        }

        [[nodiscard]] QString artist() const {
            return value(ArtistRole).toString();
        }

        [[nodiscard]] bool isValidArtist() const {
            const auto &artistData = value(ArtistRole);
            return artistData.isValid() && !artistData.toString().isEmpty();
        }

        [[nodiscard]] QStringList genres() const {
            return value(GenreRole).toStringList();
        }

        [[nodiscard]] QUrl albumArtURI() const {
            return value(ImageUrlRole).toUrl();
        }

        [[nodiscard]] bool isSingleDiscAlbum() const {
            return value(IsSingleDiscAlbumRole).toBool();
        }

        [[nodiscard]] bool isValid() const {
            return !isEmpty();
        }
    };

    using ListAlbumMetadataField = QList<AlbumMetadataField>;

    class ArtistMetadataField : public MusicMetadataField {
    public:
        using MusicMetadataField::MusicMetadataField;

        [[nodiscard]] QString name() const {
            return value(TitleRole).toString();
        }

        [[nodiscard]] qulonglong databaseId() const {
            return value(DatabaseIdRole).toULongLong();
        }

        [[nodiscard]] QUrl artistArtURI() const {
            return value(ImageUrlRole).toUrl();
        }
    };

    using ListArtistMetadataField = QList<ArtistMetadataField>;

    class GenreMetadataField : public MusicMetadataField {
    public:
        using MusicMetadataField::MusicMetadataField;

        [[nodiscard]] QString title() const {
            return value(TitleRole).toString();
        }
    };

    using ListGenreMetadataField = QList<GenreMetadataField>;

    struct EntryMetadata {
        MusicMetadataField musicMetadata;
        QString title;
        QUrl url;

        [[nodiscard]] bool isValid() const {
            return !musicMetadata.isEmpty() || !title.isEmpty() || !url.isEmpty();
        }
    };

    using EntryMetadataList = QList<EntryMetadata>;
};

Q_DECLARE_METATYPE(MetadataFields::MusicMetadataField)
Q_DECLARE_METATYPE(MetadataFields::TrackMetadataField)
Q_DECLARE_METATYPE(MetadataFields::AlbumMetadataField)
Q_DECLARE_METATYPE(MetadataFields::ArtistMetadataField)
Q_DECLARE_METATYPE(MetadataFields::GenreMetadataField)
Q_DECLARE_METATYPE(MetadataFields::ListTrackMetadataField)
Q_DECLARE_METATYPE(MetadataFields::ListAlbumMetadataField)
Q_DECLARE_METATYPE(MetadataFields::ListArtistMetadataField)
Q_DECLARE_METATYPE(MetadataFields::ListGenreMetadataField)
Q_DECLARE_METATYPE(MetadataFields::EntryMetadata)
Q_DECLARE_METATYPE(MetadataFields::EntryMetadataList)

#endif // METADATAFIELDS_H
