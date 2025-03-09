#include "database/dbschema.h"

#include "database/sqlquery.h"

#include <QSqlQuery>
#include <QSqlError>

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
        SqlQuery query{db, "PRAGMA foreign_keys = OFF;"};
        if (!query.exec()) {
            qWarning() << "Failed to disable foreign keys: " << query.lastError().text();
            setStatus(DbStatus::DatabaseError);
            return false;
        }
    }

    {
        const QString statement = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS `Tracks` ("
            "   `TrackID` INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   `FileName` TEXT NOT NULL UNIQUE,"
            "   `Title` TEXT NOT NULL,"
            "   `ArtistName` TEXT,"
            "   `AlbumTitle` TEXT,"
            "   `AlbumArtistName` TEXT,"
            "   `TrackNumber` INTEGER,"
            "   `DiscNumber` INTEGER,"
            "   `Duration` INTEGER NOT NULL,"
            "   `Genre` TEXT,"
            "   `Performer` TEXT,"
            "   `Composer` TEXT,"
            "   `Lyricist` TEXT,"
            "   `Year` INTEGER,"
            "   `Channels` INTEGER,"
            "   `Bitrate` INTEGER,"
            "   `SampleRate` INTEGER,"
            "   `HasEmbeddedCover` INTEGER,"
            "   `DateAdded` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP),"
            "   `TrackHash` TEXT UNIQUE"
            ");"
        );

        SqlQuery query{db, statement};
        if (!query.exec()) {
            qWarning() << "Failed to create Tracks table: " << query.lastError().text();
            setStatus(DbStatus::DatabaseError);
            return false;
        }
    }

    {
        const QString statement = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS `Playlists` ("
            "   `PlaylistID` INTEGER PRIMARY KEY AUTOINCREMENT,"
            "   `PlaylistName` TEXT NOT NULL UNIQUE,"
            "   `DateCreated` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP),"
            "   `LastModified` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP)"
            ");"
        );

        SqlQuery query{db, statement};
        if (!query.exec()) {
            qWarning() << "Failed to create Playlists table: " << query.lastError().text();
            setStatus(DbStatus::DatabaseError);
            return false;
        }
    }

    {
        const QString statement = QStringLiteral(
            "CREATE TABLE IF NOT EXISTS `PlaylistTracks` ("
            "   `PlaylistID` INTEGER NOT NULL,"
            "   `TrackID` INTEGER NOT NULL,"
            "   `TrackIndex` INTEGER NOT NULL,"
            "   `DateAdded` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP),"
            "   PRIMARY KEY (`PlaylistID`, `TrackID`),"
            "   FOREIGN KEY (`PlaylistID`) REFERENCES `Playlists`(`PlaylistID`)"
            "       ON DELETE CASCADE,"
            "   FOREIGN KEY (`TrackID`) REFERENCES `Tracks`(`TrackID`)"
            "       ON DELETE CASCADE"
            ");"
        );

        SqlQuery query{db, statement};
        if (!query.exec()) {
            qWarning() << "Failed to create PlaylistTracks table: " << query.lastError().text();
            setStatus(DbStatus::DatabaseError);
            return false;
        }
    }

    // Create indexes
    {
        const QStringList indexStatements = {
            "CREATE INDEX IF NOT EXISTS idx_tracks_filename ON `Tracks`(`FileName`);",
            "CREATE INDEX IF NOT EXISTS idx_playlist_tracks_order ON `PlaylistTracks`(`PlaylistID`, `TrackIndex`);"
        };

        for (const QString& statement : indexStatements) {
            SqlQuery query{db, statement};
            if (!query.exec()) {
                qWarning() << "Failed to create index: " << query.lastError().text();
                setStatus(DbStatus::DatabaseError);
                return false;
            }
        }
    }

    // Create triggers
    {
        const QStringList triggerStatements = {
            "CREATE TRIGGER IF NOT EXISTS update_playlist_modified_insert "
            "AFTER INSERT ON `PlaylistTracks` "
            "BEGIN "
            "   UPDATE `Playlists` "
            "   SET `LastModified` = (CURRENT_TIMESTAMP) "
            "   WHERE `PlaylistID` = NEW.PlaylistID; "
            "END;",

            "CREATE TRIGGER IF NOT EXISTS update_playlist_modified_delete "
            "AFTER DELETE ON `PlaylistTracks` "
            "BEGIN "
            "   UPDATE `Playlists` "
            "   SET `LastModified` = (CURRENT_TIMESTAMP) "
            "   WHERE `PlaylistID` = OLD.PlaylistID; "
            "END;",

            "CREATE TRIGGER IF NOT EXISTS update_playlist_modified_update "
            "AFTER UPDATE ON `PlaylistTracks` "
            "BEGIN "
            "   UPDATE `Playlists` "
            "   SET `LastModified` = (CURRENT_TIMESTAMP) "
            "   WHERE `PlaylistID` = NEW.PlaylistID; "
            "END;"
        };

        for (const QString& statement : triggerStatements) {
            SqlQuery query{db, statement};
            if (!query.exec()) {
                qWarning() << "Failed to create trigger: " << query.lastError().text();
                setStatus(DbStatus::DatabaseError);
                return false;
            }
        }
    }

    {
        SqlQuery query{db, "PRAGMA foreign_keys = ON;"};
        if (!query.exec()) {
            qWarning() << "Failed to enable foreign keys: " << query.lastError().text();
            setStatus(DbStatus::DatabaseError);
            return false;
        }
    }

    return m_status == DbStatus::Ok;
}
