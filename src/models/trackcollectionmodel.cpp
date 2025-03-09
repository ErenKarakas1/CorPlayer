#include "models/trackcollectionmodel.hpp"

#include "library/library.hpp"

#include <QTime>

TrackCollectionModel::TrackCollectionModel(Library* library, QObject* parent)
    : QAbstractListModel(parent), m_library(library) {
    connect(m_library, &Library::trackAdded, this, &TrackCollectionModel::onTrackAdded);
    connect(m_library, &Library::trackModified, this, &TrackCollectionModel::onTrackModified);
    connect(m_library, &Library::trackRemoved, this, &TrackCollectionModel::onTrackRemoved);
    refresh();
}

int TrackCollectionModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_tracks.size());
}

QVariant TrackCollectionModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid() || index.row() >= m_tracks.size()) return {};

    const auto& track = m_tracks[index.row()];

    switch (role) {
    case IdRole:
        return track.get(Metadata::Fields::DatabaseId);
    case TitleRole:
        return track.get(Metadata::Fields::Title);
    case ArtistRole:
        return track.get(Metadata::Fields::Artist);
    case AlbumRole:
        return track.get(Metadata::Fields::Album);
    case AlbumArtistRole:
        return track.get(Metadata::Fields::AlbumArtist);
    case UrlRole:
        return track.get(Metadata::Fields::ResourceUrl);
    case TrackNumberRole:
        return track.get(Metadata::Fields::TrackNumber);
    case DiscNumberRole:
        return track.get(Metadata::Fields::DiscNumber);
    case DurationRole:
        return track.get(Metadata::Fields::Duration);
    case DurationStringRole: {
        const QTime duration = track.get(Metadata::Fields::Duration).toTime();
        if (duration.hour() == 0) {
            return duration.toString(QStringLiteral("mm:ss"));
        }
        return duration.toString();
    }
    case YearRole:
        return track.get(Metadata::Fields::Year);
    case GenreRole:
        return track.get(Metadata::Fields::Genre);
    default:
        return {};
    }
}

QHash<int, QByteArray> TrackCollectionModel::roleNames() const {
    auto roles = QAbstractItemModel::roleNames();

    // clang-format off
    roles[IdRole]             = "trackId";
    roles[TitleRole]          = "title";
    roles[ArtistRole]         = "artist";
    roles[AlbumRole]          = "album";
    roles[AlbumArtistRole]    = "albumArtist";
    roles[UrlRole]            = "resourceUrl";
    roles[TrackNumberRole]    = "trackNumber";
    roles[DiscNumberRole]     = "discNumber";
    roles[DurationRole]       = "duration";
    roles[DurationStringRole] = "durationString";
    roles[YearRole]           = "year";
    roles[GenreRole]          = "genre";
    // clang-format on

    return roles;
}

quint64 TrackCollectionModel::getTrackId(const int index) const {
    if (index < 0 || index >= m_tracks.size()) return 0;
    return m_tracks[index].get(Metadata::Fields::DatabaseId).toULongLong();
}

QUrl TrackCollectionModel::getTrackUrl(const int index) const {
    if (index < 0 || index >= m_tracks.size()) return {};
    return m_tracks[index].get(Metadata::Fields::ResourceUrl).toUrl();
}

void TrackCollectionModel::refresh() {
    beginResetModel();
    m_tracks = m_library->trackDatabase().getTracks();

    std::ranges::sort(m_tracks, [](const Metadata::TrackFields& a, const Metadata::TrackFields& b) {
        // First by album artist
        const QString albumArtistA = a.get(Metadata::Fields::AlbumArtist).toString();
        const QString albumArtistB = b.get(Metadata::Fields::AlbumArtist).toString();
        if (albumArtistA != albumArtistB) {
            return albumArtistA < albumArtistB;
        }

        // Then by album
        const QString albumA = a.get(Metadata::Fields::Album).toString();
        const QString albumB = b.get(Metadata::Fields::Album).toString();
        if (albumA != albumB) {
            return albumA < albumB;
        }

        // Then by disc number
        const int discA = a.get(Metadata::Fields::DiscNumber).toInt();
        const int discB = b.get(Metadata::Fields::DiscNumber).toInt();
        if (discA != discB) {
            return discA < discB;
        }

        // Finally by track number
        return a.get(Metadata::Fields::TrackNumber).toInt() < b.get(Metadata::Fields::TrackNumber).toInt();
    });

    endResetModel();
}

void TrackCollectionModel::onTrackAdded(const quint64 id, const Metadata::TrackFields& track) {
    int insertIndex = 0;
    for (; insertIndex < m_tracks.size(); ++insertIndex) {
        const auto& existingTrack = m_tracks[insertIndex];

        const QString albumArtistA = track.get(Metadata::Fields::AlbumArtist).toString();
        const QString albumArtistB = existingTrack.get(Metadata::Fields::AlbumArtist).toString();
        if (albumArtistA < albumArtistB) break;
        if (albumArtistA > albumArtistB) continue;

        const QString albumA = track.get(Metadata::Fields::Album).toString();
        const QString albumB = existingTrack.get(Metadata::Fields::Album).toString();
        if (albumA < albumB) break;
        if (albumA > albumB) continue;

        const int discNumA = track.get(Metadata::Fields::DiscNumber).toInt();
        const int discNumB = existingTrack.get(Metadata::Fields::DiscNumber).toInt();
        if (discNumA < discNumB) break;
        if (discNumA > discNumB) continue;

        const int trackNumA = track.get(Metadata::Fields::TrackNumber).toInt();
        const int trackNumB = existingTrack.get(Metadata::Fields::TrackNumber).toInt();
        if (trackNumA < trackNumB) break;
    }

    beginInsertRows({}, insertIndex, insertIndex);
    m_tracks.insert(insertIndex, track);
    endInsertRows();
}

void TrackCollectionModel::onTrackModified(const quint64 id, const Metadata::TrackFields& track) {
    const int index = findTrackIndex(id);
    if (index < 0) return;

    // Check if the track needs to move due to sorting
    const int newIndex = [this, &track, index] {
        m_tracks.removeAt(index);
        int newPos = 0;
        for (; newPos < m_tracks.size(); ++newPos) {
            const auto& existingTrack = m_tracks[newPos];

            const QString albumArtistA = track.get(Metadata::Fields::AlbumArtist).toString();
            const QString albumArtistB = existingTrack.get(Metadata::Fields::AlbumArtist).toString();
            if (albumArtistA < albumArtistB) break;
            if (albumArtistA > albumArtistB) continue;

            const QString albumA = track.get(Metadata::Fields::Album).toString();
            const QString albumB = existingTrack.get(Metadata::Fields::Album).toString();
            if (albumA < albumB) break;
            if (albumA > albumB) continue;

            const int discNumA = track.get(Metadata::Fields::DiscNumber).toInt();
            const int discNumB = existingTrack.get(Metadata::Fields::DiscNumber).toInt();
            if (discNumA < discNumB) break;
            if (discNumA > discNumB) continue;

            const int trackNumA = track.get(Metadata::Fields::TrackNumber).toInt();
            const int trackNumB = existingTrack.get(Metadata::Fields::TrackNumber).toInt();
            if (trackNumA < trackNumB) break;
        }
        m_tracks.insert(newPos, track);
        return newPos;
    }();

    if (newIndex == index) {
        Q_EMIT dataChanged(createIndex(index, 0), createIndex(index, 0));
    } else {
        beginMoveRows({}, index, index, {}, newIndex + (newIndex > index ? 1 : 0));
        endMoveRows();
    }
}

void TrackCollectionModel::onTrackRemoved(const quint64 id) {
    const int index = findTrackIndex(id);
    if (index < 0) return;

    beginRemoveRows({}, index, index);
    m_tracks.removeAt(index);
    endRemoveRows();
}

int TrackCollectionModel::findTrackIndex(const quint64 id) const {
    for (int i = 0; i < m_tracks.size(); ++i) {
        if (m_tracks[i].get(Metadata::Fields::DatabaseId).toULongLong() == id) {
            return i;
        }
    }
    return -1;
}
