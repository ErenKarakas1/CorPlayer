#ifndef METADATA_HPP
#define METADATA_HPP

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

    struct PlaylistRecord {
        quint64 id;
        QString name;
        QList<quint64> trackIds;
        QDateTime dateCreated;
        QDateTime lastModified;
    };
};

Q_DECLARE_METATYPE(Metadata::TrackFields)
Q_DECLARE_METATYPE(Metadata::EntryFields)
Q_DECLARE_METATYPE(Metadata::EntryFieldsList)
Q_DECLARE_METATYPE(Metadata::PlaylistFields)
Q_DECLARE_METATYPE(Metadata::PlaylistRecord)

#endif // METADATA_HPP
