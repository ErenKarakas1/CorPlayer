#include "trackswatchdog.h"

#include "database/trackdatabase.h"
#include "filescanner.h"

#include <utility>

class TracksWatchdogPrivate {
public:
    FileScanner m_fileScanner;

    TrackDatabase m_trackDb;

    QList<Metadata::TrackFields> m_newTracks;
};

TracksWatchdog::TracksWatchdog(QObject* parent) : QObject(parent), tw(std::make_unique<TracksWatchdogPrivate>()) {}

TracksWatchdog::~TracksWatchdog() = default;

void TracksWatchdog::initDatabase(const std::shared_ptr<DbConnectionPool>& dbConnectionPool) const {
    tw->m_trackDb.initialize(DbConnection{dbConnectionPool});
}

void TracksWatchdog::addNewUrl(const QUrl& entryUrl, const PlayerUtils::PlaylistEntryType databaseIdType) {
    if (databaseIdType == PlayerUtils::Track || databaseIdType == PlayerUtils::FileName) {
        const auto newId = tw->m_trackDb.fetchTrackIdFromFileName(entryUrl);

        if (!newId) {
            addTrackByLocalFile(entryUrl);
            return;
        }

        const auto existingTrack = tw->m_trackDb.fetchTrackFromId(newId);

        if (existingTrack.isValid()) {
            Q_EMIT trackHasChanged(existingTrack);
        }
    }
}

void TracksWatchdog::addTrackByLocalFile(const QUrl& fileName) {
    if (fileName.isLocalFile() || fileName.scheme().isEmpty()) {
        auto newTrack = tw->m_fileScanner.scanFile(fileName);

        if (newTrack.isValid()) {
            tw->m_newTracks.append(newTrack);
            // TODO temp
            auto tracks = QList{newTrack};
            tw->m_trackDb.insertTracks(tracks);
            Q_EMIT trackHasChanged(newTrack);
        }
    }
}
