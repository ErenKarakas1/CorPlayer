#include "trackswatchdog.h"
#include "filescanner.h"

#include <QDebug>

class TracksWatchdogPrivate {
public:
    FileScanner m_fileScanner;
};

TracksWatchdog::TracksWatchdog(QObject *parent) : QObject(parent), tw(std::make_unique<TracksWatchdogPrivate>()) {}

TracksWatchdog::~TracksWatchdog() = default;

void TracksWatchdog::addNewUrl(const QUrl &entryUrl, PlayerUtils::PlaylistEntryType databaseIdType) {
    qDebug() << "TracksWatchdog::addNewUrl" << entryUrl;
    auto newTrack = tw->m_fileScanner.scanFile(entryUrl);

    qDebug() << "TracksWatchdog::addNewUrl" << newTrack;
    Q_EMIT trackHasChanged(newTrack);
}
