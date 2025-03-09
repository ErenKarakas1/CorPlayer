#ifndef PLAYLISTDATABASE_H
#define PLAYLISTDATABASE_H

#include "database/basedatabase.h"
#include "metadata.hpp"

class PlaylistDatabase : public BaseDatabase {
public:
    [[nodiscard]] QList<Metadata::PlaylistRecord> getPlaylists() const;
    [[nodiscard]] Metadata::PlaylistRecord getPlaylist(const QString& name) const;
    [[nodiscard]] Metadata::PlaylistRecord getPlaylist(quint64 id) const;

    [[nodiscard]] bool savePlaylist(const Metadata::PlaylistRecord& record) const;
    [[nodiscard]] bool updatePlaylist(const Metadata::PlaylistRecord& record) const;
    [[nodiscard]] bool removePlaylist(quint64 id) const;

    [[nodiscard]] bool addTracksToPlaylist(quint64 playlistId, const QList<quint64>& trackIds) const;
    [[nodiscard]] bool removeTrackFromPlaylist(quint64 playlistId, quint64 trackId) const;
    [[nodiscard]] bool reorderPlaylistTracks(quint64 playlistId, const QList<quint64>& trackIds) const;
    [[nodiscard]] QList<quint64> getPlaylistTracks(quint64 id) const;
};

#endif // PLAYLISTDATABASE_H
