#ifndef TRACKDATABASE_H
#define TRACKDATABASE_H

#include "basedatabase.h"
#include "metadata.hpp"

class TrackDatabase : public BaseDatabase {
public:
    using TrackFieldsList = QList<Metadata::TrackFields>;

    [[nodiscard]] TrackFieldsList getTracks() const;
    bool insertTracks(TrackFieldsList& tracks) const;
    bool updateTracks(TrackFieldsList& tracks) const;
    [[nodiscard]] bool deleteTrack(qulonglong trackId) const;
    bool deleteTracks(TrackFieldsList& tracks) const;
    [[nodiscard]] qulonglong fetchTrackIdFromFileName(const QUrl& fileName) const;
    [[nodiscard]] Metadata::TrackFields fetchTrackFromId(qulonglong trackId) const;

private:
    [[nodiscard]] int trackCount() const;
    bool insertTrack(Metadata::TrackFields& track) const;
    [[nodiscard]] bool updateTrack(const Metadata::TrackFields& track) const;
};

#endif // TRACKDATABASE_H
