#ifndef TRACKDATABASE_H
#define TRACKDATABASE_H

#include "database/basedatabase.h"
#include "metadata.hpp"

class TrackDatabase : public BaseDatabase {
public:
    using TrackFieldsList = QList<Metadata::TrackFields>;

    [[nodiscard]] TrackFieldsList getTracks() const;
    bool insertTracks(TrackFieldsList& tracks) const;
    bool updateTracks(TrackFieldsList& tracks) const;
    [[nodiscard]] bool deleteTrack(quint64 trackId) const;
    bool deleteTracks(TrackFieldsList& tracks) const;

    [[nodiscard]] QHash<QUrl, quint64> fetchTrackIdsFromFileNames(const QList<QUrl>& fileNames) const;
    [[nodiscard]] quint64 fetchTrackIdFromFileName(const QUrl& fileName) const;
    [[nodiscard]] Metadata::TrackFields fetchTrackFromId(quint64 trackId) const;

private:
    bool insertTrack(Metadata::TrackFields& track) const;
    [[nodiscard]] bool updateTrack(const Metadata::TrackFields& track) const;
};

#endif // TRACKDATABASE_H
