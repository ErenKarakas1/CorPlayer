#ifndef TRACKSWATCHDOG_H
#define TRACKSWATCHDOG_H

#include "database/dbconnectionpool.h"
#include "metadatafields.h"
#include "playerutils.h"

#include <QObject>

class TracksWatchdogPrivate;

class TracksWatchdog : public QObject {
    Q_OBJECT

public:
    using ListTrackMetadata = MetadataFields::ListTrackMetadataField;
    using TrackMetadata = MetadataFields::TrackMetadataField;

    explicit TracksWatchdog(QObject *parent = nullptr);
    ~TracksWatchdog() override;

Q_SIGNALS:
    void trackHasChanged(const TracksWatchdog::TrackMetadata &track);

public Q_SLOTS:
    void initDatabase(const std::shared_ptr<DbConnectionPool> &dbConnectionPool) const;
    void addNewUrl(const QUrl &entryUrl, PlayerUtils::PlaylistEntryType databaseIdType);
    void addTrackByLocalFile(const QUrl &fileName);

private:
    std::unique_ptr<TracksWatchdogPrivate> tw;
};

#endif // TRACKSWATCHDOG_H
