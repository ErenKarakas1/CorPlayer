#include "trackdatabase.h"

#include "sqlquery.h"
#include "sqltransaction.h"

#include <QSqlError>

namespace {

QString getTrackColumns() {
    static const QString columns = QStringLiteral(
        "`FileName`, `Title`, `ArtistName`, `AlbumTitle`, `AlbumArtistName`, `TrackNumber`, "
        "`DiscNumber`, `Duration`, `Genre`, `Performer`, `Composer`, `Lyricist`, `Year`, `Channels`, `Bitrate`, "
        "`SampleRate`, `HasEmbeddedCover`, `TrackHash`");

    return columns;
}

QString getTrackColumnBinds() {
    static const QString values = QStringLiteral(
        ":fileName, :title, :artist, :album, :albumArtist, :trackNumber, :discNumber, :duration, :genre, "
        ":performer, :composer, :lyricist, :year, :channels, :bitRate, :sampleRate, :hasEmbeddedCover, :trackHash");

    return values;
}

Metadata::TrackFields insertTrackMetadata(const SqlQuery& query) {
    Metadata::TrackFields track;

    using enum Metadata::Fields;
    track.insert(DatabaseId, query.value(0));
    track.insert(ResourceUrl, query.value(1));
    track.insert(Title, query.value(2));
    track.insert(Artist, query.value(3));
    track.insert(Album, query.value(4));
    track.insert(AlbumArtist, query.value(5));
    track.insert(TrackNumber, query.value(6));
    track.insert(DiscNumber, query.value(7));
    track.insert(Duration, query.value(8));
    track.insert(Genre, query.value(9));
    track.insert(Performer, query.value(10));
    track.insert(Composer, query.value(11));
    track.insert(Lyricist, query.value(12));
    track.insert(Year, query.value(13));
    track.insert(Channels, query.value(14));
    track.insert(BitRate, query.value(15));
    track.insert(SampleRate, query.value(16));
    track.insert(HasEmbeddedCover, query.value(17));
    track.insert(CoverImage, QVariant{QLatin1String("image://cover/") + query.value(1).toUrl().toLocalFile()}); // TODO: handle this better when introducing caching
    track.insert(Hash, query.value(18));

    return track;
}

std::map<QString, QVariant> getTrackBindings(const Metadata::TrackFields& track) {
    using enum Metadata::Fields;
    return {{QStringLiteral(":fileName"), track.get(ResourceUrl)},
            {QStringLiteral(":title"), track.get(Title)},
            {QStringLiteral(":artist"), track.get(Artist)},
            {QStringLiteral(":album"), track.get(Album)},
            {QStringLiteral(":albumArtist"), track.get(AlbumArtist)},
            {QStringLiteral(":trackNumber"), track.get(TrackNumber)},
            {QStringLiteral(":discNumber"), track.get(DiscNumber)},
            {QStringLiteral(":duration"), track.get(Duration)},
            {QStringLiteral(":genre"), track.get(Genre)},
            {QStringLiteral(":performer"), track.get(Performer)},
            {QStringLiteral(":composer"), track.get(Composer)},
            {QStringLiteral(":lyricist"), track.get(Lyricist)},
            {QStringLiteral(":year"), track.get(Year)},
            {QStringLiteral(":channels"), track.get(Channels)},
            {QStringLiteral(":bitRate"), track.get(BitRate)},
            {QStringLiteral(":sampleRate"), track.get(SampleRate)},
            {QStringLiteral(":hasEmbeddedCover"), track.get(HasEmbeddedCover)},
            {QStringLiteral(":trackHash"), track.get(Hash)}};
}

} // namespace

TrackDatabase::TrackFieldsList TrackDatabase::getTracks() const {
    const QString statement = QStringLiteral("SELECT `TrackID`, %1 FROM Tracks;").arg(getTrackColumns());
    SqlQuery query{db(), statement};

    if (!query.exec()) {
        return {};
    }

    const int rowCount = trackCount();

    if (rowCount < 1) {
        return {};
    }

    TrackFieldsList tracks;
    tracks.reserve(rowCount);

    while (query.next()) {
        tracks.append(insertTrackMetadata(query));
    }

    return tracks;
}

bool TrackDatabase::insertTracks(TrackFieldsList& tracks) const {
    if (tracks.isEmpty()) {
        return true;
    }

    SqlTransaction transaction{db()};

    for (auto& track : tracks) {
        if (!track.contains(Metadata::Fields::DatabaseId)) {
            insertTrack(track);
        }
    }

    return transaction.commit();
}

bool TrackDatabase::updateTracks(TrackFieldsList& tracks) const {
    if (tracks.isEmpty()) {
        return true;
    }

    SqlTransaction transaction{db()};

    for (auto& track : tracks) {
        if (track.contains(Metadata::Fields::DatabaseId)) {
            if (!updateTrack(track)) {
                qWarning() << "Failed to update track: " << track.get(Metadata::Fields::Title).toString();
            }
        }
    }

    return transaction.commit();
}

bool TrackDatabase::deleteTrack(const qulonglong trackId) const {
    const QString statement = QStringLiteral("DELETE FROM `Tracks` WHERE `TrackID` = :trackId;");
    SqlQuery query{db(), statement};
    query.bindNumericValue(QStringLiteral(":trackId"), trackId);

    return query.exec();
}

bool TrackDatabase::deleteTracks(TrackFieldsList& tracks) const {
    if (tracks.isEmpty()) {
        return true;
    }

    SqlTransaction transaction{db()};

    int deletedCount = 0;

    for (const auto& track : tracks) {
        if (track.contains(Metadata::Fields::DatabaseId) &&
            deleteTrack(track.get(Metadata::Fields::DatabaseId).toULongLong())) {
            ++deletedCount;
        }
    }

    return transaction.commit() && deletedCount == tracks.size();
}

qulonglong TrackDatabase::fetchTrackIdFromFileName(const QUrl& fileName) const {
    const QString statement = QStringLiteral("SELECT `TrackID` FROM `Tracks` WHERE `FileName` = :fileName;");
    SqlQuery query{db(), statement};
    query.bindStringValue(QStringLiteral(":fileName"), fileName.toString());

    if (!query.exec()) {
        return {};
    }

    if (query.next()) {
        return query.value(0).toULongLong();
    }

    return {};
}

Metadata::TrackFields TrackDatabase::fetchTrackFromId(const qulonglong trackId) const {
    const QString statement =
        QStringLiteral("SELECT `TrackID`, %1 FROM `Tracks` WHERE `TrackID` = :trackId;").arg(getTrackColumns());
    SqlQuery query{db(), statement};
    query.bindNumericValue(QStringLiteral(":trackId"), trackId);

    if (!query.exec()) {
        return {};
    }

    if (query.next()) {
        return insertTrackMetadata(query);
    }

    return {};
}

int TrackDatabase::trackCount() const {
    const QString statement = QStringLiteral("SELECT COUNT(*) FROM `Tracks`;");
    SqlQuery query{db(), statement};

    if (!query.exec()) {
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}

bool TrackDatabase::insertTrack(Metadata::TrackFields& track) const {
    const QString statement =
        QStringLiteral("INSERT INTO `Tracks` (%1) VALUES (%2);").arg(getTrackColumns(), getTrackColumnBinds());

    SqlQuery query{db(), statement};

    const auto bindings = getTrackBindings(track);
    for (const auto& [key, value] : bindings) {
        query.bindValue(key, value);
    }

    if (!query.exec()) {
        qWarning() << "Failed to insert track: " << query.lastError().text() << "\nLast query: " << query.lastQuery();
        return false;
    }

    track.insert(Metadata::Fields::DatabaseId, query.lastInsertId().toULongLong());

    return true;
}

bool TrackDatabase::updateTrack(const Metadata::TrackFields& track) const {
    const QString statement = QStringLiteral(
        "UPDATE `Tracks` SET `Title` = :title, `ArtistName` = :artist, `AlbumTitle` = :album, `Genre` = :genre, "
        "`Duration` = :duration, `TrackNumber` = :track_number, `Year` = :year, `FileName` = :fileName "
        "WHERE `TrackID` = :trackId;");
    SqlQuery query{db(), statement};
    query.bindNumericValue(QStringLiteral(":trackId"), track.get(Metadata::Fields::DatabaseId).toULongLong());

    const auto bindings = getTrackBindings(track);
    for (const auto& [key, value] : bindings) {
        query.bindValue(key, value);
    }

    return query.exec();
}
