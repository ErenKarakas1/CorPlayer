#include "models/playlistcollectionmodel.hpp"

#include "library/library.hpp"

PlaylistCollectionModel::PlaylistCollectionModel(Library* library, QObject* parent)
    : QAbstractListModel(parent), m_library(library) {
    connect(m_library, &Library::playlistModified, this, &PlaylistCollectionModel::onPlaylistModified);
    loadPlaylists();
}

int PlaylistCollectionModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_playlists.size());
}

QVariant PlaylistCollectionModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid() || index.row() >= m_playlists.size()) return {};

    const auto& playlist = m_playlists[index.row()];

    switch (role) {
    case PlaylistIdRole:
        return playlist.id;
    case NameRole:
        return playlist.name;
    case TrackCountRole:
        return playlist.trackIds.size();
    default:
        return {};
    }
}

QHash<int, QByteArray> PlaylistCollectionModel::roleNames() const {
    auto roles = QAbstractItemModel::roleNames();

    // clang-format off
    roles[PlaylistIdRole] = "playlistId";
    roles[NameRole]       = "name";
    roles[TrackCountRole] = "trackCount";
    // clang-format on

    return roles;
}

void PlaylistCollectionModel::createPlaylist(const QString& name) {
    Metadata::PlaylistRecord record;
    record.name = name;
    if (m_library->playlistDatabase().savePlaylist(record)) {
        loadPlaylists();
    }
}

void PlaylistCollectionModel::renamePlaylist(const int index, const QString& name) {
    if (index < 0 || index >= m_playlists.size()) return;

    auto playlist = m_playlists[index];
    playlist.name = name;

    if (m_library->playlistDatabase().updatePlaylist(playlist)) {
        loadPlaylists();
    }
}

void PlaylistCollectionModel::removePlaylist(const int index) {
    if (index < 0 || index >= m_playlists.size()) return;

    const quint64 id = m_playlists[index].id;
    if (m_library->playlistDatabase().removePlaylist(id)) {
        loadPlaylists();
    }
}

QList<quint64> PlaylistCollectionModel::getPlaylistTracks(const int index) const {
    if (index < 0 || index >= m_playlists.size()) return {};
    return m_library->playlistDatabase().getPlaylistTracks(m_playlists[index].id);
}

void PlaylistCollectionModel::onPlaylistModified(const quint64 id) {
    Q_UNUSED(id) // TODO: use id to update only the modified playlist
    loadPlaylists();
}

void PlaylistCollectionModel::loadPlaylists() {
    beginResetModel();
    m_playlists = m_library->playlistDatabase().getPlaylists();
    endResetModel();
}
