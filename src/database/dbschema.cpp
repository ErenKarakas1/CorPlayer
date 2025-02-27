#include "dbschema.h"

#include <QSqlQuery>

DbSchema::DbSchema(const DbConnection& dbConnection, QObject* parent) : QObject(parent), m_status{DbStatus::Ok} {
    if (!dbConnection.isValid()) {
        setStatus(DbStatus::ConnectionError);
        return;
    }

    if (!createSchema(dbConnection.db())) {
        setStatus(DbStatus::BrokenSchemaError);
    }
}

DbSchema::DbStatus DbSchema::status() const {
    return m_status;
}

void DbSchema::setStatus(const DbStatus status) {
    if (m_status == status) return;

    m_status = status;
    Q_EMIT statusChanged(m_status);
}

bool DbSchema::createSchema(const QSqlDatabase& db) {
    {
        QSqlQuery query(db);
        const auto result = query.exec(QStringLiteral("PRAGMA foreign_keys = OFF;"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to disable foreign keys.");
            setStatus(DbStatus::DatabaseError);
        }
    }

    {
        QSqlQuery query(db);
        const auto result = query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS `Tracks` ("
                                                      "`TrackID` INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                      "`FileName` TEXT NOT NULL UNIQUE, "
                                                      "`Title` TEXT NOT NULL, "
                                                      "`ArtistName` TEXT, "
                                                      "`AlbumTitle` TEXT, "
                                                      "`AlbumArtistName` TEXT, "
                                                      "`TrackNumber` INTEGER, "
                                                      "`DiscNumber` INTEGER, "
                                                      "`Duration` INTEGER NOT NULL, "
                                                      "`Genre` TEXT, "
                                                      "`Performer` TEXT, "
                                                      "`Composer` TEXT, "
                                                      "`Lyricist` TEXT, "
                                                      "`Year` INTEGER, "
                                                      "`Channels` INTEGER, "
                                                      "`Bitrate` INTEGER, "
                                                      "`SampleRate` INTEGER, "
                                                      "`HasEmbeddedCover` INTEGER, "
                                                      "`TrackHash` TEXT UNIQUE)"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to create table \"Tracks\".");
            setStatus(DbStatus::DatabaseError);
        }
    }

    {
        QSqlQuery query(db);
        const auto result = query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS `Playlists` ("
                                                      "`PlaylistID` INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                      "`PlaylistName` TEXT NOT NULL UNIQUE)"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to create table \"Playlists\".");
            setStatus(DbStatus::DatabaseError);
        }
    }

    {
        QSqlQuery query(db);
        const auto result = query.exec(
            QStringLiteral("CREATE TABLE IF NOT EXISTS `PlaylistTracks` ("
                           "`PlaylistID` INTEGER NOT NULL, "
                           "`TrackID` INTEGER NOT NULL, "
                           "`TrackIndex` INTEGER NOT NULL, "
                           "FOREIGN KEY(`PlaylistID`) REFERENCES `Playlists`(`PlaylistID`) ON DELETE CASCADE, "
                           "FOREIGN KEY(`TrackID`) REFERENCES `Tracks`(`TrackID`) ON DELETE CASCADE, "
                           "PRIMARY KEY(`PlaylistID`, `TrackID`))"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to create table \"PlaylistTracks\".");
            setStatus(DbStatus::DatabaseError);
        }
    }

    {
        QSqlQuery query(db);
        const auto result = query.exec(QStringLiteral("PRAGMA foreign_keys = ON;"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to enable foreign keys.");
            setStatus(DbStatus::DatabaseError);
        }
    }

    {
        QSqlQuery query(db);
        const auto result =
            query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS `TrackIndex` ON `Tracks`(`TrackHash`)"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to create index \"TrackIndex\".");
            setStatus(DbStatus::DatabaseError);
        }
    }

    {
        QSqlQuery query(db);
        const auto result = query.exec(
            QStringLiteral("CREATE INDEX IF NOT EXISTS `PlaylistIndex` ON `Playlists`(`PlaylistID`, `PlaylistName`)"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to create index \"PlaylistIndex\".");
            setStatus(DbStatus::DatabaseError);
        }
    }

    {
        QSqlQuery query(db);
        const auto result = query.exec(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS `PlaylistTrackIndex` ON `PlaylistTracks`(`PlaylistID`, `TrackIndex`)"));

        if (!result) {
            qWarning() << QStringLiteral("Failed to create index \"PlaylistTrackIndex\".");
            setStatus(DbStatus::DatabaseError);
        }
    }

    return m_status == DbStatus::Ok;
}
