#include "../playlist/playlistmodel.hpp"

#include "../playerutils.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QUrl>

#include <algorithm>
#include <utility>

PlaylistModel::PlaylistModel(QObject* parent) : QAbstractListModel(parent) {};

PlaylistModel::~PlaylistModel() = default;

int PlaylistModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(m_entries.size());
}

QHash<int, QByteArray> PlaylistModel::roleNames() const {
    auto roles = QAbstractItemModel::roleNames();

    roles[IsValidRole] = "isValid";
    roles[DatabaseIdRole] = "databaseId";
    roles[TitleRole] = "title";
    roles[ArtistRole] = "artist";
    roles[AlbumRole] = "album";
    roles[ResourceUrlRole] = "resourceUrl";
    roles[TrackNumberRole] = "trackNumber";
    roles[DiscNumberRole] = "discNumber";
    roles[DurationRole] = "duration";
    roles[DurationStringRole] = "durationString";
    roles[ElementTypeRole] = "elementType";
    roles[IsPlayingRole] = "isPlaying";

    return roles;
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};

    const auto& entry = m_entries[index.row()];

    switch (role) {
    case IsValidRole:
        return entry.m_isValid;
    case TitleRole:
        if (m_trackFields[index.row()].contains(Metadata::Fields::Title)) {
            return m_trackFields[index.row()].get(Metadata::Fields::Title).toString();
        }
        return m_trackFields[index.row()].get(Metadata::Fields::ResourceUrl).toUrl().fileName();
    case DurationRole:
        return m_trackFields[index.row()].get(Metadata::Fields::Duration).toTime();
    case DurationStringRole: {
        const QTime trackDuration = m_trackFields[index.row()].get(Metadata::Fields::Duration).toTime();
        if (trackDuration.hour() == 0) {
            return trackDuration.toString(QStringLiteral("mm:ss"));
        }
        return trackDuration;
    }
    case ElementTypeRole:
        return entry.m_entryType;
    case IsPlayingRole:
        return entry.m_isPlaying;
    default:
        const auto& trackFields = m_trackFields[index.row()];
        const auto roleEnum = static_cast<Metadata::Fields>(role);
        if (trackFields.contains(roleEnum)) {
            return trackFields.get(roleEnum);
        }
        return {};
    }
}

bool PlaylistModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;
    if (index.row() < 0 || index.row() >= m_entries.size()) return false;

    bool modelModified = false;
    auto& entry = m_entries[index.row()];

    switch (role) {
    case TitleRole:
        entry.m_title = value.toString();
        modelModified = true;
        break;
    case ArtistRole:
        entry.m_artist = value.toString();
        modelModified = true;
        break;
    case IsPlayingRole:
        entry.m_isPlaying = static_cast<PlayState>(value.toInt());
        modelModified = true;
        break;
    default:
        break;
    }

    if (modelModified) {
        m_trackFields[index.row()].insert(static_cast<Metadata::Fields>(role), value);
        Q_EMIT dataChanged(index, index, {role});
    }

    return modelModified;
}

bool PlaylistModel::removeRows(const int row, const int count, const QModelIndex& parent) {
    if (row < 0 || row + count > m_entries.size()) return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; ++i) {
        m_entries.removeAt(i);
        m_trackFields.removeAt(i);
    }
    endRemoveRows();

    return true;
}

bool PlaylistModel::moveRows(const QModelIndex& sourceParent, const int sourceRow, int count,
                             const QModelIndex& destinationParent, const int destinationChild) {
    if (sourceRow < 0 || sourceRow + count > m_entries.size() || destinationChild < 0 ||
        destinationChild > m_entries.size()) {
        return false;
    }
    if (sourceRow == destinationChild) return true;

    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    while (count-- > 0) {
        m_entries.move(sourceRow, destinationChild);
        m_trackFields.move(sourceRow, destinationChild);
    }
    endMoveRows();

    return true;
}

void PlaylistModel::clearPlaylist() {
    if (m_entries.isEmpty()) return;

    beginRemoveRows(QModelIndex(), 0, static_cast<int>(m_entries.count()) - 1);
    m_entries.clear();
    m_trackFields.clear();
    endRemoveRows();
}

void PlaylistModel::enqueueRestoredEntries(const QVariantList& newEntries) {
    if (newEntries.isEmpty()) return;

    const int start = static_cast<int>(m_entries.size());

    beginInsertRows(QModelIndex(), start, start + newEntries.size() - 1);
    for (const QVariant& entry : newEntries) {
        auto fields = entry.toStringList();
        if (fields.size() < 7) continue;

        PlaylistEntry newEntry {
            fields[0].toULongLong(),
            fields[1],
            fields[2],
            fields[3],
            fields.size() == 8 ? QUrl{fields[7]} : QUrl{},
            fields[4].toInt(),
            fields[5].toInt(),
            static_cast<PlayerUtils::PlaylistEntryType>(fields[6].toInt())
        };
        newEntry.m_isValid = false;

        m_entries.push_back(newEntry);
        m_trackFields.push_back({});

        if (newEntry.m_resourceUrl.isValid()) {
            auto entryURL = newEntry.m_resourceUrl;
            if (entryURL.isLocalFile()) {
                auto entryString = entryURL.toLocalFile();
                const QFileInfo newTrackFile(entryString);
                if (newTrackFile.exists()) {
                    m_entries.last().m_isValid = true;
                    Q_EMIT addNewEntry(0, entryString, PlayerUtils::FileName);
                } else if (newEntry.m_title.isEmpty()) {
                    Q_EMIT addNewEntry(0, entryString, PlayerUtils::FileName);
                } else {
                    Q_EMIT addTrackByName(newEntry.m_title, newEntry.m_artist, newEntry.m_album, newEntry.m_trackNumber,
                                          newEntry.m_discNumber);
                }
            } else {
                m_entries.last().m_isValid = true;
            }
        } else {
            Q_EMIT addTrackByName(newEntry.m_title, newEntry.m_artist, newEntry.m_album, newEntry.m_trackNumber,
                                  newEntry.m_discNumber);
        }
    }
    endInsertRows();
}

QVariantList PlaylistModel::getEntriesForRestore() const {
    QVariantList result;

    for (int trackIndex = 0; trackIndex < m_entries.size(); ++trackIndex) {
        const auto& entry = m_entries[trackIndex];
        if (entry.m_isValid) {
            QStringList entryData;
            const auto& entryFields = m_trackFields[trackIndex];

            entryData.push_back(QString::number(entryFields.get(Metadata::Fields::DatabaseId).toULongLong()));
            entryData.push_back(entryFields.get(Metadata::Fields::Title).toString());
            entryData.push_back(entryFields.get(Metadata::Fields::Artist).toString());

            if (entryFields.contains(Metadata::Fields::Album)) {
                entryData.push_back(entryFields.get(Metadata::Fields::Album).toString());
            } else {
                entryData.push_back({});
            }

            if (entryFields.contains(Metadata::Fields::TrackNumber)) {
                entryData.push_back(entryFields.get(Metadata::Fields::TrackNumber).toString());
            } else {
                entryData.push_back({});
            }

            if (entryFields.contains(Metadata::Fields::DiscNumber)) {
                entryData.push_back(QString::number(entryFields.get(Metadata::Fields::DiscNumber).toInt()));
            } else {
                entryData.push_back({});
            }
            entryData.push_back(QString::number(entry.m_entryType));
            // TODO: toUrl + toString or no?
            entryData.push_back(entryFields.get(Metadata::Fields::ResourceUrl).toString());

            result.push_back(QVariant(entryData));
        }
    }
    return result;
}

void PlaylistModel::enqueueOneEntry(const Metadata::EntryFields& entryData, const int insertAt) {
    enqueueMultipleEntries({entryData}, insertAt);
}

void PlaylistModel::enqueueMultipleEntries(const Metadata::EntryFieldsList& entriesData, const int insertAt) {
    int validEntries = 0;
    for (const auto& entry : entriesData) {
        if (entry.isValid()) ++validEntries;
    }

    if (validEntries == 0) return;

    m_entries.reserve(m_entries.size() + validEntries);
    m_trackFields.reserve(m_entries.size() + validEntries);

    int i = insertAt < 0 || insertAt > m_entries.size() ? m_entries.size() : insertAt;
    beginInsertRows(QModelIndex(), i, i + validEntries - 1);

    for (const auto& entry : entriesData) {
        if (!entry.isValid()) continue;

        const auto resourceUrl =
            entry.url.isValid() ? entry.url : entry.trackFields.get(Metadata::Fields::ResourceUrl).toUrl();

        if (!entry.trackFields.contains(Metadata::Fields::DatabaseId) && resourceUrl.isValid()) {
            auto newEntry = PlaylistEntry{resourceUrl};
            newEntry.m_entryType = PlayerUtils::FileName;

            m_entries.insert(i, std::move(newEntry));
            m_trackFields.insert(i, {});
        } else {
            const auto& data = entry.trackFields;
            m_entries.insert(
                i, PlaylistEntry{data.get(Metadata::Fields::DatabaseId).toULongLong(),
                                 data.get(Metadata::Fields::Title).toString(),
                                 data.get(Metadata::Fields::ElementType).value<PlayerUtils::PlaylistEntryType>()});

            switch (data.get(Metadata::Fields::ElementType).value<PlayerUtils::PlaylistEntryType>()) {
            case PlayerUtils::Track:
            case PlayerUtils::FileName:
                m_trackFields.insert(i, data);
                break;
            default:
                m_trackFields.insert(i, {});
            }
        }

        if (resourceUrl.isValid()) {
            auto type = PlayerUtils::FileName;
            if (entry.trackFields.contains(Metadata::Fields::ElementType)) {
                type = static_cast<PlayerUtils::PlaylistEntryType>(
                    entry.trackFields.get(Metadata::Fields::ElementType).toInt());
            }
            Q_EMIT addNewUrl(resourceUrl, type);

        } else {
            Q_EMIT addNewEntry(
                entry.trackFields.get(Metadata::Fields::DatabaseId).toULongLong(), entry.title,
                entry.trackFields.get(Metadata::Fields::ElementType).value<PlayerUtils::PlaylistEntryType>());
        }
        ++i;
    }
    endInsertRows();
}

void PlaylistModel::tracksListAdded(const qulonglong newDatabaseId, const QString& entryTitle,
                                    const PlayerUtils::PlaylistEntryType databaseIdType,
                                    const TrackFieldsList& tracks) {
    if (tracks.isEmpty()) {
        return;
    }

    for (int playListIndex = 0; playListIndex < m_entries.size(); ++playListIndex) {
        auto& entry = m_entries[playListIndex];
        if (entry.m_entryType != databaseIdType) continue;
        if (entry.m_title != entryTitle) continue;
        if (newDatabaseId != 0 && entry.m_dbId != newDatabaseId) continue;

        beginRemoveRows(QModelIndex(), playListIndex, playListIndex);
        m_entries.removeAt(playListIndex);
        m_trackFields.removeAt(playListIndex);
        endRemoveRows();

        beginInsertRows(QModelIndex(), playListIndex, playListIndex - 1 + static_cast<int>(tracks.size()));

        for (int trackIndex = 0; trackIndex < tracks.size(); ++trackIndex) {
            auto newEntry = PlaylistEntry{tracks[trackIndex]};
            newEntry.m_entryType = PlayerUtils::Track;
            m_entries.insert(playListIndex + trackIndex, newEntry);
            m_trackFields.insert(playListIndex + trackIndex, tracks[trackIndex]);
        }

        endInsertRows();
    }
}

void PlaylistModel::trackChanged(const Metadata::TrackFields& track) {
    for (int i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];

        if (entry.m_entryType != PlayerUtils::Artist && entry.m_isValid) {
            if (entry.m_resourceUrl.isValid() && track.get(Metadata::Fields::ResourceUrl).toUrl() != entry.m_resourceUrl) {
                continue;
            }

            if (!entry.m_resourceUrl.isValid() &&
                (entry.m_dbId == 0 || track.get(Metadata::Fields::DatabaseId).toULongLong() != entry.m_dbId)) {
                continue;
            }

            const auto& trackData = m_trackFields[i];

            if (!trackData.isEmpty()) {
                bool sameData = true;
                for (auto oneKeyIterator = track.data.constKeyValueBegin();
                     oneKeyIterator != track.data.constKeyValueEnd(); ++oneKeyIterator) {
                    if (trackData.get(oneKeyIterator->first) != oneKeyIterator->second) {
                        sameData = false;
                        break;
                    }
                }
                if (sameData) {
                    continue;
                }
            }

            m_trackFields[i] = track;

            Q_EMIT dataChanged(index(i, 0), index(i, 0), {});
            continue;
        }

        if (entry.m_entryType != PlayerUtils::Artist && !entry.m_isValid && !entry.m_resourceUrl.isValid()) {
            if (track.contains(Metadata::Fields::Title) && track.get(Metadata::Fields::Title).toString() != entry.m_title) {
                continue;
            }

            if (track.contains(Metadata::Fields::Album) && track.get(Metadata::Fields::Album).toString() != entry.m_album) {
                continue;
            }

            if (track.contains(Metadata::Fields::TrackNumber) &&
                track.get(Metadata::Fields::TrackNumber).toInt() != entry.m_trackNumber) {
                continue;
            }

            if (track.contains(Metadata::Fields::DiscNumber) &&
                track.get(Metadata::Fields::DiscNumber).toInt() != entry.m_discNumber) {
                continue;
            }

            m_trackFields[i] = track;
            entry.m_dbId = track.get(Metadata::Fields::DatabaseId).toULongLong();
            entry.m_isValid = true;

            Q_EMIT dataChanged(index(i, 0), index(i, 0), {});

            break;
        }

        if (entry.m_entryType != PlayerUtils::Artist && !entry.m_isValid && entry.m_resourceUrl.isValid()) {
            if (track.get(Metadata::Fields::ResourceUrl).toUrl() != entry.m_resourceUrl) {
                continue;
            }

            m_trackFields[i] = track;
            entry.m_dbId = track.get(Metadata::Fields::DatabaseId).toULongLong();
            entry.m_isValid = true;

            Q_EMIT dataChanged(index(i, 0), index(i, 0), {});
            break;
        }
    }
}

void PlaylistModel::trackRemoved(const qulonglong trackId) {
    for (int i = 0; i < m_entries.size(); ++i) {
        auto& oneEntry = m_entries[i];

        if (oneEntry.m_isValid) {
            if (oneEntry.m_dbId == trackId) {
                oneEntry.m_isValid = false;
                oneEntry.m_title = m_trackFields[i].get(Metadata::Fields::Title).toString();
                oneEntry.m_artist = m_trackFields[i].get(Metadata::Fields::Artist).toString();
                oneEntry.m_album = m_trackFields[i].get(Metadata::Fields::Album).toString();
                oneEntry.m_trackNumber = m_trackFields[i].get(Metadata::Fields::TrackNumber).toInt();
                oneEntry.m_discNumber = m_trackFields[i].get(Metadata::Fields::DiscNumber).toInt();

                Q_EMIT dataChanged(index(i, 0), index(i, 0), {});
            }
        }
    }
}

void PlaylistModel::trackInError(const QUrl& sourceInError, const QMediaPlayer::Error playerError) {
    Q_UNUSED(playerError)

    for (int i = 0; i < m_entries.size(); ++i) {
        auto& oneTrack = m_entries[i];
        if (oneTrack.m_isValid) {
            const auto& oneTrackData = m_trackFields.at(i);

            if (oneTrackData.get(Metadata::Fields::ResourceUrl).toUrl() == sourceInError) {
                oneTrack.m_isValid = false;
                Q_EMIT dataChanged(index(i, 0), index(i, 0), {Metadata::Fields::IsValid});
            }
        }
    }
}

PlaylistEntry::PlaylistEntry(const qulonglong id, QString title, QString artist, QString album, QUrl resourceUrl,
                             const int trackNumber, const int discNumber, const PlaylistModel::EntryType entryType)
    : m_title(std::move(title)), m_album(std::move(album)), m_artist(std::move(artist)),
      m_resourceUrl(std::move(resourceUrl)), m_trackNumber(trackNumber), m_discNumber(discNumber), m_dbId(id),
      m_entryType(entryType) {}

PlaylistEntry::PlaylistEntry(const Metadata::TrackFields& track)
    : m_title(track.get(Metadata::Fields::Title).toString()), m_album(track.get(Metadata::Fields::Album).toString()),
      m_artist(track.get(Metadata::Fields::Artist).toString()),
      m_resourceUrl(track.get(Metadata::Fields::ResourceUrl).toUrl()),
      m_trackNumber(track.get(Metadata::Fields::TrackNumber).toInt()),
      m_discNumber(track.get(Metadata::Fields::DiscNumber).toInt()),
      m_dbId(track.get(Metadata::Fields::DatabaseId).toULongLong()), m_isValid(true), m_entryType(PlayerUtils::Track) {}

PlaylistEntry::PlaylistEntry(QUrl fileName) : m_resourceUrl(std::move(fileName)) {}

PlaylistEntry::PlaylistEntry(const quint64 id, const QString& entryTitle, const PlaylistModel::EntryType type)
    : m_title(entryTitle), m_dbId(id), m_isValid(true), m_entryType(type) {}
