#include "trackswatchdog.h"

#include "filescanner.h"

class TracksWatchdogPrivate {
public:
    FileScanner m_fileScanner;
};

TracksWatchdog::TracksWatchdog(QObject *parent) : QObject(parent), tw(std::make_unique<TracksWatchdogPrivate>()) {}

TracksWatchdog::~TracksWatchdog() = default;

void TracksWatchdog::addNewUrl(const QUrl &entryUrl, PlayerUtils::PlaylistEntryType databaseIdType) {
    const auto newTrack = tw->m_fileScanner.scanFile(entryUrl);
    Q_EMIT trackHasChanged(newTrack);
}
