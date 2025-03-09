#include "models/playlistmodel.hpp"

#include "library/library.hpp"

#include <QList>
#include <QUrl>

#include <utility>

class PlaylistEntry {
public:
    PlaylistEntry() = default;

    explicit PlaylistEntry(const Metadata::TrackFields& trackFields)
        : m_dbId(trackFields.get(Metadata::Fields::DatabaseId).toULongLong()),
          m_resourceUrl(trackFields.get(Metadata::Fields::ResourceUrl).toUrl()), m_isValid(true),
          m_entryType(PlayerUtils::Track) {}

    explicit PlaylistEntry(QUrl resourceUrl)
        : m_resourceUrl(std::move(resourceUrl)), m_entryType(PlayerUtils::FileName) {}

    quint64 m_dbId = 0;
    QUrl m_resourceUrl{};
    bool m_isValid = false;
    PlayerUtils::PlaylistEntryType m_entryType = PlayerUtils::Unknown;
    PlaylistModel::PlayState m_isPlaying = PlaylistModel::NotPlaying;
};

class PlaylistModelPrivate {
public:
    Library* m_library = nullptr;
    QList<PlaylistEntry> m_entries;
    QList<Metadata::TrackFields> m_trackFields;
};

PlaylistModel::PlaylistModel(Library* library, QObject* parent)
    : QAbstractListModel(parent), p(std::make_unique<PlaylistModelPrivate>()) {
    p->m_library = library;
    connect(p->m_library, &Library::trackModified, this, &PlaylistModel::onTrackModified);
    connect(p->m_library, &Library::trackRemoved, this, &PlaylistModel::onTrackRemoved);
}

PlaylistModel::~PlaylistModel() = default;

int PlaylistModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(p->m_entries.size());
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};

    const auto& entry = p->m_entries[index.row()];
    const auto& fields = p->m_trackFields[index.row()];

    switch (role) {
    case IsValidRole:
        return entry.m_isValid;
    case DatabaseIdRole:
        return entry.m_dbId;
    case TitleRole:
        if (fields.contains(Metadata::Fields::Title)) {
            return fields.get(Metadata::Fields::Title);
        }
        return entry.m_resourceUrl.fileName();
    case ResourceUrlRole:
        return entry.m_resourceUrl;
    case DurationRole:
        return fields.get(Metadata::Fields::Duration);
    case DurationStringRole: {
        const QTime duration = fields.get(Metadata::Fields::Duration).toTime();
        if (duration.hour() == 0) {
            return duration.toString(QStringLiteral("mm:ss"));
        }
        return duration.toString();
    }
    case ElementTypeRole:
        return QVariant::fromValue(entry.m_entryType);
    case IsPlayingRole:
        return entry.m_isPlaying;
    default:
        if (fields.contains(static_cast<Metadata::Fields>(role))) {
            return fields.get(static_cast<Metadata::Fields>(role));
        }
        return {};
    }
}

bool PlaylistModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;

    auto& entry = p->m_entries[index.row()];
    auto& fields = p->m_trackFields[index.row()];

    switch (role) {
    case IsPlayingRole:
        entry.m_isPlaying = static_cast<PlayState>(value.toInt());
        Q_EMIT dataChanged(index, index, {role});
        return true;
    default:
        if (static_cast<Metadata::Fields>(role) >= Metadata::Fields::Title) {
            fields.insert(static_cast<Metadata::Fields>(role), value);
            Q_EMIT dataChanged(index, index, {role});
            return true;
        }
        return false;
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const {
    auto roles = QAbstractItemModel::roleNames();

    // clang-format off
    roles[IsValidRole]        = "isValid";
    roles[DatabaseIdRole]     = "databaseId";
    roles[TitleRole]          = "title";
    roles[ArtistRole]         = "artist";
    roles[AlbumRole]          = "album";
    roles[ResourceUrlRole]    = "resourceUrl";
    roles[TrackNumberRole]    = "trackNumber";
    roles[DiscNumberRole]     = "discNumber";
    roles[DurationRole]       = "duration";
    roles[DurationStringRole] = "durationString";
    roles[ElementTypeRole]    = "elementType";
    roles[IsPlayingRole]      = "isPlaying";
    // clang-format on

    return roles;
}

void PlaylistModel::clearPlaylist() {
    if (p->m_entries.isEmpty()) return;

    beginRemoveRows({}, 0, static_cast<int>(p->m_entries.count()) - 1);
    p->m_entries.clear();
    p->m_trackFields.clear();
    endRemoveRows();
}

void PlaylistModel::enqueueRestoredEntries(const QVariantList& newEntries) {
    if (newEntries.isEmpty()) return;

    const int start = static_cast<int>(p->m_entries.size());

    beginInsertRows(QModelIndex(), start, start + newEntries.size() - 1);
    for (const QVariant& entry : newEntries) {
        auto fields = entry.toStringList();
        if (fields.size() < 2) continue;

        const quint64 dbId = fields[0].toULongLong();
        const QUrl resourceUrl = QUrl(fields[1]);

        if (dbId != 0) {
            auto trackFields = p->m_library->getTrackById(dbId);
            if (trackFields.isValid()) {
                p->m_entries.push_back(PlaylistEntry{trackFields});
                p->m_trackFields.push_back(trackFields);
                continue;
            }
        }

        if (resourceUrl.isValid()) {
            auto newEntry = PlaylistEntry{resourceUrl};
            newEntry.m_entryType = PlayerUtils::FileName;
            p->m_entries.push_back(newEntry);
            p->m_trackFields.push_back({});
            Q_EMIT addNewUrl(resourceUrl, PlayerUtils::FileName);
        }
    }
    endInsertRows();
}

void PlaylistModel::enqueueMultipleEntries(const Metadata::EntryFieldsList& newEntries, const int insertAt) {
    if (newEntries.isEmpty()) return;

    int validEntries = 0;
    for (const auto& entry : newEntries) {
        if (entry.isValid()) ++validEntries;
    }

    if (validEntries == 0) return;
    p->m_entries.reserve(p->m_entries.size() + validEntries);
    p->m_trackFields.reserve(p->m_entries.size() + validEntries);

    int i = (insertAt < 0 || insertAt > p->m_entries.size()) ? p->m_entries.size() : insertAt;

    beginInsertRows({}, i, i + validEntries - 1);

    for (const auto& entry : newEntries) {
        if (!entry.isValid()) continue;

        const auto resourceUrl =
            entry.url.isValid() ? entry.url : entry.trackFields.get(Metadata::Fields::ResourceUrl).toUrl();

        if (!entry.trackFields.contains(Metadata::Fields::DatabaseId) && resourceUrl.isValid()) {
            auto newEntry = PlaylistEntry(resourceUrl);
            newEntry.m_entryType = PlayerUtils::FileName;
            p->m_entries.insert(i, std::move(newEntry));
            p->m_trackFields.insert(i, Metadata::TrackFields{});
        } else {
            const auto& data = entry.trackFields;
            p->m_entries.insert(i, PlaylistEntry{data});
            p->m_trackFields.insert(i, data);
        }

        if (resourceUrl.isValid()) {
            auto type = PlayerUtils::FileName;
            if (entry.trackFields.contains(Metadata::Fields::ElementType)) {
                type = static_cast<PlayerUtils::PlaylistEntryType>(
                    entry.trackFields.get(Metadata::Fields::ElementType).toInt());
            }
            Q_EMIT addNewUrl(resourceUrl, type);
        }
        // else {
        //     Q_EMIT addNewEntry(
        //         entryData.trackFields.get(Metadata::Fields::DatabaseId).toULongLong(), entryData.title,
        //         entryData.trackFields.get(Metadata::Fields::ElementType).value<PlayerUtils::PlaylistEntryType>());
        // }
        ++i;
    }

    endInsertRows();
}

void PlaylistModel::loadTracksFromIds(const QList<quint64>& trackIds) {
    clearPlaylist();
    if (trackIds.isEmpty()) return;

    p->m_entries.reserve(trackIds.size());
    p->m_trackFields.reserve(trackIds.size());

    beginInsertRows(QModelIndex(), 0, static_cast<int>(trackIds.size()) - 1);
    for (const quint64 id : trackIds) {
        auto trackFields = p->m_library->getTrackById(id);
        if (trackFields.isValid()) {
            p->m_entries.push_back(PlaylistEntry{trackFields});
            p->m_trackFields.push_back(trackFields);
        } else {
            PlaylistEntry entry;
            entry.m_dbId = id;
            entry.m_entryType = PlayerUtils::Track;
            p->m_entries.push_back(entry);
            p->m_trackFields.push_back({});
        }
    }
    endInsertRows();
}

QVariantList PlaylistModel::getEntriesForRestore() const {
    QVariantList result;

    for (const auto& entry : p->m_entries) {
        if (!entry.m_isValid) continue;

        QStringList entryData;
        entryData.append(QString::number(entry.m_dbId));
        entryData.append(entry.m_resourceUrl.toString());
        result.append(QVariant(entryData));
    }

    return result;
}

void PlaylistModel::onTrackModified(const quint64 id, const Metadata::TrackFields& track) {
    for (int i = 0; i < p->m_entries.size(); ++i) {
        auto& entry = p->m_entries[i];
        if (entry.m_dbId == id || entry.m_resourceUrl == track.get(Metadata::Fields::ResourceUrl).toUrl()) {
            entry.m_isValid = true;
            entry.m_dbId = id;
            entry.m_entryType = PlayerUtils::Track;
            p->m_trackFields[i] = track;
            Q_EMIT dataChanged(index(i), index(i));
        }
    }
}

void PlaylistModel::onTrackRemoved(const quint64 id) {
    for (int i = 0; i < p->m_entries.size(); ++i) {
        auto& entry = p->m_entries[i];
        if (entry.m_dbId == id) {
            entry.m_isValid = false;
            Q_EMIT dataChanged(index(i), index(i), {IsValidRole});
        }
    }
}
