#include "database/playlistdatabase.h"

#include "database/sqlquery.h"
#include "database/sqltransaction.h"

#include <QSqlError>

QList<Metadata::PlaylistRecord> PlaylistDatabase::getPlaylists() const {
    const auto db = this->db();

    int count = 0;
    {
        const QString countStatement = QStringLiteral("SELECT COUNT(*) FROM `Playlists`;");
        SqlQuery query{db, countStatement};
        if (query.exec() && query.next()) {
            count = query.value(0).toInt();
            if (count == 0) return {};
        }
    }

    const QString statement = QStringLiteral("SELECT `PlaylistID`, `PlaylistName`, `DateCreated`, `LastModified` "
                                             "FROM `Playlists` ORDER BY `DateCreated` DESC;");
    SqlQuery query{db, statement};

    if (!query.exec()) return {};

    QList<Metadata::PlaylistRecord> playlists;
    playlists.reserve(count);

    const QString trackStatement = QStringLiteral(
        "SELECT `TrackID` FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId ORDER BY `TrackIndex` ASC;");
    SqlQuery trackQuery{db, trackStatement};

    const QString countStatement =
        QStringLiteral("SELECT COUNT(*) FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId;");
    SqlQuery countQuery{db, countStatement};

    while (query.next()) {
        Metadata::PlaylistRecord rec{.id = query.value(0).toULongLong(),
                                     .name = query.value(1).toString(),
                                     .trackIds = {},
                                     .dateCreated = query.value(2).toDateTime(),
                                     .lastModified = query.value(3).toDateTime()};

        trackQuery.bindValue(QStringLiteral(":playlistId"), rec.id);
        countQuery.bindValue(QStringLiteral(":playlistId"), rec.id);

        if (countQuery.exec() && countQuery.next()) {
            const int trackCount = countQuery.value(0).toInt();
            if (trackCount == 0) continue;
            rec.trackIds.reserve(trackCount);
        }

        if (trackQuery.exec()) {
            while (trackQuery.next()) {
                rec.trackIds.append(trackQuery.value(0).toULongLong());
            }
        }
        playlists.append(rec);
    }

    return playlists;
}

Metadata::PlaylistRecord PlaylistDatabase::getPlaylist(const QString& name) const {
    const auto db = this->db();

    const QString statement = QStringLiteral("SELECT `PlaylistID`, `DateCreated`, `LastModified` "
                                             "FROM `Playlists` WHERE `PlaylistName` = :name;");
    SqlQuery query{db, statement};
    query.bindStringValue(QStringLiteral(":name"), name);

    if (!query.exec() || !query.next()) return {};

    Metadata::PlaylistRecord rec{.id = query.value(0).toULongLong(),
                                 .name = name,
                                 .trackIds = {},
                                 .dateCreated = query.value(1).toDateTime(),
                                 .lastModified = query.value(2).toDateTime()};

    const QString trackStatement = QStringLiteral(
        "SELECT `TrackID` FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId ORDER BY `TrackIndex` ASC;");
    SqlQuery trackQuery{db, trackStatement};
    trackQuery.bindValue(QStringLiteral(":playlistId"), rec.id);

    const QString countStatement =
        QStringLiteral("SELECT COUNT(*) FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId;");
    SqlQuery countQuery{db, countStatement};
    countQuery.bindValue(QStringLiteral(":playlistId"), rec.id);

    if (countQuery.exec() && countQuery.next()) {
        const int trackCount = countQuery.value(0).toInt();
        if (trackCount == 0) return rec;
        rec.trackIds.reserve(trackCount);
    }

    if (trackQuery.exec()) {
        while (trackQuery.next()) {
            rec.trackIds.append(trackQuery.value(0).toULongLong());
        }
    }

    return rec;
}

Metadata::PlaylistRecord PlaylistDatabase::getPlaylist(const quint64 id) const {
    const auto db = this->db();

    const QString statement = QStringLiteral("SELECT `PlaylistName`, `DateCreated`, `LastModified` "
                                             "FROM `Playlists` WHERE `PlaylistID` = :id;");
    SqlQuery query{db, statement};
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec() || !query.next()) return {};

    Metadata::PlaylistRecord rec{.id = id,
                                 .name = query.value(0).toString(),
                                 .trackIds = {},
                                 .dateCreated = query.value(1).toDateTime(),
                                 .lastModified = query.value(2).toDateTime()};

    const QString trackStatement = QStringLiteral(
        "SELECT `TrackID` FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId ORDER BY `TrackIndex` ASC;");
    SqlQuery trackQuery{db, trackStatement};
    trackQuery.bindValue(QStringLiteral(":playlistId"), id);

    const QString countStatement =
        QStringLiteral("SELECT COUNT(*) FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId;");
    SqlQuery countQuery{db, countStatement};
    countQuery.bindValue(QStringLiteral(":playlistId"), rec.id);

    if (countQuery.exec() && countQuery.next()) {
        const int trackCount = countQuery.value(0).toInt();
        if (trackCount == 0) return rec;
        rec.trackIds.reserve(trackCount);
    }

    if (trackQuery.exec()) {
        while (trackQuery.next()) {
            rec.trackIds.append(trackQuery.value(0).toULongLong());
        }
    }

    return rec;
}

bool PlaylistDatabase::savePlaylist(const Metadata::PlaylistRecord& record) const {
    auto db = this->db();
    SqlTransaction transaction{db};

    const QString statement = QStringLiteral("INSERT INTO `Playlists` (`PlaylistName`) VALUES (:name);");
    SqlQuery query{db, statement};
    query.bindStringValue(QStringLiteral(":name"), record.name);

    if (!query.exec()) {
        qWarning() << "Failed to save playlist: " << query.lastError().text() << "\nLast query: " << query.lastQuery();
        db.rollback();
        return false;
    }

    const quint64 playlistId = query.lastInsertId().toULongLong();

    if (record.trackIds.isEmpty()) {
        return transaction.commit();
    }

    const QString trackStatement = QStringLiteral("INSERT INTO `PlaylistTracks` (`PlaylistID`, `TrackID`, "
                                                  "`TrackIndex`) VALUES (:playlistId, :trackId, :trackIndex);");
    SqlQuery trackQuery{db, trackStatement};
    trackQuery.bindValue(QStringLiteral(":playlistId"), playlistId);

    for (int i = 0; i < record.trackIds.size(); ++i) {
        trackQuery.bindValue(QStringLiteral(":trackId"), record.trackIds.at(i));
        trackQuery.bindValue(QStringLiteral(":trackIndex"), i);

        if (!trackQuery.exec()) {
            qWarning() << "Failed to insert playlist track: " << trackQuery.lastError().text()
                       << "\nLast query: " << trackQuery.lastQuery();
            db.rollback();
            return false;
        }
    }

    return transaction.commit();
}

bool PlaylistDatabase::updatePlaylist(const Metadata::PlaylistRecord& record) const {
    const auto db = this->db();
    SqlTransaction transaction{db};

    const QString statement = QStringLiteral("UPDATE `Playlists` SET `PlaylistName` = :name WHERE `PlaylistID` = :id;");
    SqlQuery query{db, statement};
    query.bindStringValue(QStringLiteral(":name"), record.name);
    query.bindValue(QStringLiteral(":id"), record.id);

    if (!query.exec()) {
        qWarning() << "Failed to update playlist: " << query.lastError().text()
                   << "\nLast query: " << query.lastQuery();
        return false;
    }

    const QList<quint64> currentTracks = getPlaylistTracks(record.id);

    if (currentTracks != record.trackIds) {
        const QString deleteStatement = QStringLiteral("DELETE FROM `PlaylistTracks` WHERE `PlaylistID` = :id;");
        SqlQuery deleteQuery{db, deleteStatement};
        deleteQuery.bindValue(QStringLiteral(":id"), record.id);

        if (!deleteQuery.exec()) {
            qWarning() << "Failed to delete playlist tracks: " << deleteQuery.lastError().text();
            return false;
        }

        // TODO: is this faster than looping
        if (!record.trackIds.isEmpty()) {
            const QString trackStatement =
                QStringLiteral("INSERT INTO `PlaylistTracks` (`PlaylistID`, `TrackID`, `TrackIndex`) VALUES (?, ?, ?)");

            SqlQuery trackQuery{db, trackStatement};

            QVariantList playlistIds;
            playlistIds.fill(record.id, record.trackIds.size());

            QVariantList trackIds;
            QVariantList trackIndices;

            trackIds.reserve(record.trackIds.size());
            trackIndices.reserve(record.trackIds.size());

            for (int i = 0; i < record.trackIds.size(); ++i) {
                trackIds << record.trackIds.at(i);
                trackIndices << i;
            }

            trackQuery.addBindValue(playlistIds);
            trackQuery.addBindValue(trackIds);
            trackQuery.addBindValue(trackIndices);

            if (!trackQuery.execBatch()) {
                qWarning() << "Failed to insert playlist tracks: " << trackQuery.lastError().text();
                return false;
            }
        }
    }

    const QString updateStatement =
        QStringLiteral("UPDATE `Playlists` SET `LastModified` = CURRENT_TIMESTAMP WHERE `PlaylistID` = :id;");
    SqlQuery updateQuery{db, updateStatement};
    updateQuery.bindValue(QStringLiteral(":id"), record.id);
    if (!updateQuery.exec()) {
        qWarning() << "Failed to update playlist last modified date: " << updateQuery.lastError().text();
        return false;
    }

    return transaction.commit();
}

bool PlaylistDatabase::removePlaylist(const quint64 id) const {
    const QString statement = QStringLiteral("DELETE FROM `PlaylistTracks` WHERE `PlaylistID` = :id;");
    SqlQuery query{db(), statement};
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        qWarning() << "Failed to remove playlist tracks: " << query.lastError().text();
        return false;
    }

    const QString deleteStatement = QStringLiteral("DELETE FROM `Playlists` WHERE `PlaylistID` = :id;");
    SqlQuery deleteQuery{db(), deleteStatement};
    deleteQuery.bindValue(QStringLiteral(":id"), id);

    if (!deleteQuery.exec()) {
        qWarning() << "Failed to remove playlist: " << deleteQuery.lastError().text();
        return false;
    }

    return true;
}

// TODO: this can be optimized but it is currently unused
bool PlaylistDatabase::addTracksToPlaylist(const quint64 playlistId, const QList<quint64>& trackIds) const {
    const QString statement = QStringLiteral("INSERT INTO `PlaylistTracks` (`PlaylistID`, `TrackID`, `TrackIndex`) "
                                             "VALUES (:playlistId, :trackId, :trackIndex);");
    SqlQuery query{db(), statement};
    query.bindValue(QStringLiteral(":playlistId"), playlistId);

    for (int i = 0; i < trackIds.size(); ++i) {
        query.bindValue(QStringLiteral(":trackId"), trackIds.at(i));
        query.bindValue(QStringLiteral(":trackIndex"), i);

        if (!query.exec()) {
            qWarning() << "Failed to add track to playlist: " << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool PlaylistDatabase::removeTrackFromPlaylist(const quint64 playlistId, const quint64 trackId) const {
    const QString statement =
        QStringLiteral("DELETE FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId AND `TrackID` = :trackId;");
    SqlQuery query{db(), statement};
    query.bindValue(QStringLiteral(":playlistId"), playlistId);
    query.bindValue(QStringLiteral(":trackId"), trackId);

    if (!query.exec()) {
        qWarning() << "Failed to remove track from playlist: " << query.lastError().text();
        return false;
    }

    const QString updateStatement = QStringLiteral(
        "UPDATE `PlaylistTracks` SET `TrackIndex` = `TrackIndex` - 1 WHERE `PlaylistID` = :playlistId AND "
        "`TrackIndex` > (SELECT `TrackIndex` FROM `PlaylistTracks` WHERE `PlaylistID` = :playlistId AND `TrackID` = "
        ":trackId);");
    SqlQuery updateQuery{db(), updateStatement};
    updateQuery.bindValue(QStringLiteral(":playlistId"), playlistId);

    if (!updateQuery.exec()) {
        qWarning() << "Failed to update playlist track indexes: " << updateQuery.lastError().text();
        return false;
    }

    return true;
}

bool PlaylistDatabase::reorderPlaylistTracks(const quint64 playlistId, const QList<quint64>& trackIds) const {
    const QString statement =
        QStringLiteral("UPDATE `PlaylistTracks` SET `TrackIndex` = :index WHERE `PlaylistID` = :id "
                       "AND `TrackID` = :trackId;");
    SqlQuery query{db(), statement};
    query.bindValue(QStringLiteral(":id"), playlistId);

    for (int i = 0; i < trackIds.size(); ++i) {
        query.bindValue(QStringLiteral(":index"), i);
        query.bindValue(QStringLiteral(":trackId"), trackIds.at(i));

        if (!query.exec()) {
            qWarning() << "Failed to reorder playlist tracks: " << query.lastError().text();
            return false;
        }
    }

    return true;
}

QList<quint64> PlaylistDatabase::getPlaylistTracks(const quint64 id) const {
    int count = 0;
    {
        const QString countStatement =
            QStringLiteral("SELECT COUNT(*) FROM `PlaylistTracks` WHERE `PlaylistID` = :id;");
        SqlQuery countQuery{db(), countStatement};
        countQuery.bindValue(QStringLiteral(":id"), id);

        if (countQuery.exec() && countQuery.next()) {
            count = countQuery.value(0).toInt();
            if (count == 0) return {};
        }
    }

    const QString statement =
        QStringLiteral("SELECT `TrackID` FROM `PlaylistTracks` WHERE `PlaylistID` = :id ORDER BY `TrackIndex` ASC;");
    SqlQuery query{db(), statement};
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        qWarning() << "Failed to get playlist tracks: " << query.lastError().text();
        return {};
    }

    QList<quint64> tracks;
    tracks.reserve(count);

    while (query.next()) {
        tracks.append(query.value(0).toULongLong());
    }

    return tracks;
}
