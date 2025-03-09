#include "database/trackdatabase.h"

#include "database/sqlquery.h"
#include "database/sqltransaction.h"

#include <QSqlError>

namespace {

QString getTrackColumns() {
    static const QString columns = QStringLiteral(
        "`FileName`, `Title`, `ArtistName`, `AlbumTitle`, `AlbumArtistName`, `TrackNumber`, "
        "`DiscNumber`, `Duration`, `Genre`, `Performer`, `Composer`, `Lyricist`, `Year`, `Channels`, `Bitrate`, "
        "`SampleRate`, `HasEmbeddedCover`, `TrackHash`, `DateAdded`");

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
    track.insert(CoverImage,
                 QVariant{"image://cover/" +
                          query.value(1).toUrl().toLocalFile()}); // TODO: handle this better when introducing caching
    track.insert(Hash, query.value(18));
    track.insert(DateAdded, query.value(19));

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
    const auto db = this->db();

    int count = 0;
    {
        const QString countStatement = QStringLiteral("SELECT COUNT(*) FROM `Tracks`;");
        SqlQuery query{db, countStatement};
        if (query.exec() && query.next()) {
            count = query.value(0).toInt();
            if (count == 0) return {};
        }
    }

    const QString statement = QStringLiteral("SELECT `TrackID`, %1 FROM Tracks;").arg(getTrackColumns());
    SqlQuery query{db, statement};

    if (!query.exec()) return {};

    TrackFieldsList tracks;
    tracks.reserve(count);

    while (query.next()) {
        tracks.append(insertTrackMetadata(query));
    }

    return tracks;
}

// Returning true does not mean that all tracks were inserted successfully
bool TrackDatabase::insertTracks(TrackFieldsList& tracks) const {
    if (tracks.isEmpty()) return true;

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

bool TrackDatabase::deleteTrack(const quint64 trackId) const {
    const QString statement = QStringLiteral("DELETE FROM `Tracks` WHERE `TrackID` = :trackId;");
    SqlQuery query{db(), statement};
    query.bindValue(QStringLiteral(":trackId"), trackId);

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

QHash<QUrl, quint64> TrackDatabase::fetchTrackIdsFromFileNames(const QList<QUrl>& fileNames) const {
    QHash<QUrl, quint64> result;

    if (fileNames.isEmpty()) return result;

    QStringList placeholders;
    placeholders.reserve(fileNames.size());
    for (int i = 0; i < fileNames.size(); ++i) {
        placeholders.append("?");
    }

    const QString statement = QStringLiteral("SELECT `TrackID`, `FileName` FROM `Tracks` WHERE `FileName` IN (%1);")
                                  .arg(placeholders.join(", "));
    SqlQuery query{db(), statement};

    for (const QUrl& fileName : fileNames) {
        query.addBindValue(fileName.toString());
    }

    if (!query.exec()) {
        qWarning() << "Failed to fetch track IDs from file names: " << query.lastError().text()
                   << "\nLast query: " << query.lastQuery();
        return result;
    }

    while (query.next()) {
        const quint64 trackId = query.value(0).toULongLong();
        const QString fileName = query.value(1).toString();
        result.emplace(QUrl{fileName}, trackId);
    }

    return result;
}

quint64 TrackDatabase::fetchTrackIdFromFileName(const QUrl& fileName) const {
    const QString statement = QStringLiteral("SELECT `TrackID` FROM `Tracks` WHERE `FileName` = :fileName;");
    SqlQuery query{db(), statement};
    query.bindStringValue(QStringLiteral(":fileName"), fileName.toString());

    if (!query.exec()) return {};

    if (query.next()) {
        return query.value(0).toULongLong();
    }

    return {};
}

Metadata::TrackFields TrackDatabase::fetchTrackFromId(const quint64 trackId) const {
    const QString statement =
        QStringLiteral("SELECT `TrackID`, %1 FROM `Tracks` WHERE `TrackID` = :trackId;").arg(getTrackColumns());
    SqlQuery query{db(), statement};
    query.bindValue(QStringLiteral(":trackId"), trackId);

    if (!query.exec()) return {};

    if (query.next()) {
        return insertTrackMetadata(query);
    }

    return {};
}

bool TrackDatabase::insertTrack(Metadata::TrackFields& track) const {
    // FIXME: Workaround to ensure that the `DateAdded` column is populated with the current time
    const QString statement =
        QStringLiteral("INSERT INTO `Tracks` (%1) VALUES (%2);")
            .arg(getTrackColumns().remove(QStringLiteral(", `DateAdded`")), getTrackColumnBinds());

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
    query.bindValue(QStringLiteral(":trackId"), track.get(Metadata::Fields::DatabaseId).toULongLong());

    const auto bindings = getTrackBindings(track);
    for (const auto& [key, value] : bindings) {
        query.bindValue(key, value);
    }

    return query.exec();
}
