#include <playlist/playlistmodel.hpp>

#include "playerutils.hpp"

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

    using enum Metadata::Fields;
    roles[IsValid] = "isValid";
    roles[DatabaseId] = "databaseId";
    roles[Title] = "title";
    roles[Duration] = "durationInt";
    roles[DurationString] = "duration";
    roles[Artist] = "artist";
    roles[AlbumArtist] = "albumArtist";
    roles[Album] = "album";
    roles[TrackNumber] = "trackNumber";
    roles[DiscNumber] = "discNumber";
    roles[Genre] = "genre";
    roles[Performer] = "performer";
    roles[Lyricist] = "lyricist";
    roles[Composer] = "composer";
    roles[Comment] = "comment";
    roles[Year] = "year";
    roles[Channels] = "channels";
    roles[BitRate] = "bitRate";
    roles[SampleRate] = "sampleRate";
    roles[IsPlaying] = "isPlaying";
    // roles[static_cast<int>(IsSingleDiscAlbum)] = "isSingleDiscAlbum";
    // roles[static_cast<int>(SecondaryText)] = "secondaryText";
    roles[CoverImage] = "coverImage";
    // roles[static_cast<int>(ShadowForImage)] = "shadowForImage";
    roles[ResourceUrl] = "resourceUrl";
    // roles[static_cast<int>(FullData)] = "trackData";
    roles[AlbumId] = "albumId";
    roles[Metadata::PlaylistFields::AlbumSection] = "albumSection";
    roles[ElementType] = "entryType";
    roles[Metadata::PlaylistFields::MetadataModifiable] = "metadataModifiableRole";
    roles[Hash] = "hash";

    return roles;
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    auto result = QVariant();

    if (!index.isValid()) {
        return result;
    }

    using enum Metadata::Fields;
    if (m_entries[index.row()].m_isValid) {
        switch (role) {
        case IsValid:
            result = m_entries[index.row()].m_isValid;
            break;
        case Metadata::PlaylistFields::IsPlaying:
            result = m_entries[index.row()].m_isPlaying;
            break;
        case ElementType:
            result = m_entries[index.row()].m_entryType;
            break;
        case Duration:
            result = m_trackFields[index.row()].get(Duration).toTime();
            break;
        case DurationString: {
            const QTime trackDuration = m_trackFields[index.row()].get(Duration).toTime();
            if (trackDuration.hour() == 0) {
                result = trackDuration.toString(QStringLiteral("mm:ss"));
            } else {
                result = trackDuration.toString();
            }
            break;
        }
        case Metadata::PlaylistFields::AlbumSection:
            result = QJsonDocument{QJsonArray{m_trackFields[index.row()].get(Album).toString(),
                                              m_trackFields[index.row()].get(AlbumArtist).toString(),
                                              m_trackFields[index.row()].get(CoverImage).toUrl().toString()}}
                         .toJson();
            break;
        case Title: {
            const auto title = m_trackFields[index.row()].get(Title).toString();
            if (title.isEmpty()) {
                result = m_trackFields[index.row()].get(ResourceUrl).toUrl().fileName();
            } else {
                result = title;
            }
            break;
        }
        case Metadata::PlaylistFields::MetadataModifiable:
            switch (m_entries[index.row()].m_entryType) {
            case PlayerUtils::Album:
            case PlayerUtils::Artist:
            case PlayerUtils::Composer:
            case PlayerUtils::Genre:
            case PlayerUtils::Unknown:
            case PlayerUtils::Lyricist:
            case PlayerUtils::Container:
            case PlayerUtils::Playlist:
                result = false;
                break;
            case PlayerUtils::FileName:
            case PlayerUtils::Track:
                result = m_trackFields[index.row()].get(ResourceUrl).toUrl().isLocalFile();
                break;
            }
            break;
        default:
            const auto& trackFields = m_trackFields[index.row()];
            const auto roleEnum = static_cast<Metadata::Fields>(role);
            if (trackFields.contains(roleEnum)) {
                result = trackFields.get(roleEnum);
            } else {
                result = {};
            }
        }
    } else {
        switch (role) {
        case IsValid:
            result = m_entries[index.row()].m_isValid;
            break;
        case Title:
            result = m_entries[index.row()].m_title;
            break;
        case Metadata::PlaylistFields::IsPlaying:
            result = m_entries[index.row()].m_isPlaying;
            break;
        case Artist:
        case AlbumArtist:
            result = m_entries[index.row()].m_artist;
            break;
        case Album:
            result = m_entries[index.row()].m_album;
            break;
        case TrackNumber:
            result = -1;
            break;
        // case IsSingleDiscAlbum:
        //     result = false;
        //     break;
        // case Qt::DisplayRole:
        //     result = m_entries[index.row()].m_title;
        //     break;
        case CoverImage:
            result = QUrl(QStringLiteral("error image")); // TODO: change to error image
            break;
        // case ShadowForImageRole:
        //     result = false;
        //     break;
        case Metadata::PlaylistFields::AlbumSection:
            result = QJsonDocument{QJsonArray{m_entries[index.row()].m_album, m_entries[index.row()].m_artist,
                                              QUrl(QStringLiteral("error image")).toString()}}
                         .toJson(); // TODO: change to error image
            break;
        default:
            result = {};
        }
    }
    return result;
}

bool PlaylistModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    bool modelModified = false;

    if (!index.isValid()) {
        return modelModified;
    }

    if (index.row() < 0 || index.row() >= m_entries.size()) {
        return modelModified;
    }

    // TODO: does this make sense now
    if (role < Metadata::Fields::IsValid ||
        role > Metadata::PlaylistFields::IsPlaying) {
        qWarning() << "hitting here";
        return modelModified;
    }

    switch (role) {
    case Metadata::PlaylistFields::IsPlaying: {
        modelModified = true;
        const auto newState = static_cast<PlayState>(value.toInt());
        m_entries[index.row()].m_isPlaying = newState;
        Q_EMIT dataChanged(index, index, {role});

        break;
    }
    case Metadata::Fields::Title: {
        modelModified = true;
        m_entries[index.row()].m_title = value.toString();
        m_trackFields[index.row()].insert(static_cast<Metadata::Fields>(role), value);
        Q_EMIT dataChanged(index, index, {role});
        break;
    }
    case Metadata::Fields::Artist: {
        m_entries[index.row()].m_artist = value.toString();
        m_trackFields[index.row()].insert(static_cast<Metadata::Fields>(role), value);
        Q_EMIT dataChanged(index, index, {role});
        break;
    }
    default:
        modelModified = false;
    }

    return modelModified;
}

bool PlaylistModel::removeRows(const int row, const int count, const QModelIndex& parent) {
    beginRemoveRows(parent, row, row + count - 1);

    for (int i = row, cpt = 0; cpt < count; ++i, ++cpt) {
        m_entries.removeAt(i);
        m_trackFields.removeAt(i);
    }
    endRemoveRows();

    return true;
}

bool PlaylistModel::moveRows(const QModelIndex& sourceParent, const int sourceRow, const int count,
                             const QModelIndex& destinationParent, const int destinationChild) {
    if (sourceParent != destinationParent) {
        return false;
    }

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
        return false;
    }

    for (auto cptItem = 0; cptItem < count; ++cptItem) {
        if (sourceRow < destinationChild) {
            m_entries.move(sourceRow, destinationChild - 1);
            m_trackFields.move(sourceRow, destinationChild - 1);
        } else {
            m_entries.move(sourceRow, destinationChild);
            m_trackFields.move(sourceRow, destinationChild);
        }
    }

    endMoveRows();

    return true;
}

void PlaylistModel::enqueueRestoredEntries(const QVariantList& newEntries) {
    if (newEntries.isEmpty()) return;

    const int tracksSize = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), tracksSize, tracksSize + newEntries.size() - 1);

    for (const auto& oneData : newEntries) {
        auto trackData = oneData.toStringList();
        if (trackData.size() != 7 && trackData.size() != 8) {
            continue;
        }

        const auto restoredId = trackData[0].toULongLong();
        const auto& restoredTitle = trackData[1];
        const auto& restoredArtist = trackData[2];
        const auto& restoredAlbum = trackData[3];
        const auto& restoredTrackNumber = trackData[4];
        const auto& restoredDiscNumber = trackData[5];
        auto restoredFileUrl = QVariant{};

        if (trackData.size() == 8) {
            restoredFileUrl = QUrl{trackData[7]};
        }

        auto m_entryType = static_cast<PlayerUtils::PlaylistEntryType>(trackData[6].toInt());
        auto newEntry = PlaylistEntry(restoredId, restoredTitle, restoredArtist, restoredAlbum, restoredFileUrl.toUrl(),
                                      restoredTrackNumber.toInt(), restoredDiscNumber.toInt(), m_entryType);

        m_entries.push_back(newEntry);
        m_trackFields.push_back({});

        if (newEntry.m_trackUrl.isValid()) {
            auto entryURL = newEntry.m_trackUrl;
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

void PlaylistModel::enqueueOneEntry(const Metadata::EntryFields& entryData, const int insertAt) {
    enqueueMultipleEntries({entryData}, insertAt);
}

void PlaylistModel::enqueueMultipleEntries(const Metadata::EntryFieldsList& entriesData, const int insertAt) {
    const int validEntries =
        std::accumulate(entriesData.cbegin(), entriesData.cend(), 0, [](const int validEntries, const auto& entryData) {
            return entryData.isValid() ? validEntries + 1 : validEntries;
        });

    if (validEntries == 0) {
        return;
    }

    m_entries.reserve(m_entries.size() + validEntries);
    m_trackFields.reserve(m_entries.size() + validEntries);

    int i = insertAt < 0 || insertAt > m_entries.size() ? m_entries.size() : insertAt;
    beginInsertRows(QModelIndex(), i, i + validEntries - 1);

    for (const auto& entryData : entriesData) {
        if (!entryData.isValid()) continue;

        const auto trackUrl =
            entryData.url.isValid() ? entryData.url : entryData.trackFields.get(Metadata::Fields::ResourceUrl).toUrl();

        if (!entryData.trackFields.contains(Metadata::Fields::DatabaseId) && trackUrl.isValid()) {
            auto newEntry = PlaylistEntry{trackUrl};
            newEntry.m_entryType = PlayerUtils::FileName;

            m_entries.insert(i, std::move(newEntry));
            m_trackFields.insert(i, {});
        } else {
            const auto& data = entryData.trackFields;
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

        if (trackUrl.isValid()) {
            auto type = PlayerUtils::FileName;
            if (entryData.trackFields.contains(Metadata::Fields::ElementType)) {
                type = static_cast<PlayerUtils::PlaylistEntryType>(
                    entryData.trackFields.get(Metadata::Fields::ElementType).toInt());
            }
            Q_EMIT addNewUrl(trackUrl, type);

        } else {
            Q_EMIT addNewEntry(
                entryData.trackFields.get(Metadata::Fields::DatabaseId).toULongLong(), entryData.title,
                entryData.trackFields.get(Metadata::Fields::ElementType).value<PlayerUtils::PlaylistEntryType>());
        }
        ++i;
    }
    endInsertRows();
}

void PlaylistModel::clearPlaylist() {
    if (m_entries.isEmpty()) return;

    beginRemoveRows({}, 0, static_cast<int>(m_entries.count()) - 1);

    m_entries.clear();
    m_trackFields.clear();

    endRemoveRows();
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
            if (entry.m_trackUrl.isValid() && track.get(Metadata::Fields::ResourceUrl).toUrl() != entry.m_trackUrl) {
                continue;
            }

            if (!entry.m_trackUrl.isValid() &&
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

        if (entry.m_entryType != PlayerUtils::Artist && !entry.m_isValid && !entry.m_trackUrl.isValid()) {
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

        if (entry.m_entryType != PlayerUtils::Artist && !entry.m_isValid && entry.m_trackUrl.isValid()) {
            if (track.get(Metadata::Fields::ResourceUrl).toUrl() != entry.m_trackUrl) {
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

PlaylistEntry::PlaylistEntry(const qulonglong id, QString title, QString artist, QString album, QUrl trackUrl,
                             const int trackNumber, const int discNumber, const PlaylistModel::EntryType entryType)
    : m_title(std::move(title)), m_album(std::move(album)), m_artist(std::move(artist)),
      m_trackUrl(std::move(trackUrl)), m_trackNumber(trackNumber), m_discNumber(discNumber), m_dbId(id),
      m_entryType(entryType) {}

PlaylistEntry::PlaylistEntry(const Metadata::TrackFields& track)
    : m_title(track.get(Metadata::Fields::Title).toString()), m_album(track.get(Metadata::Fields::Album).toString()),
      m_artist(track.get(Metadata::Fields::Artist).toString()),
      m_trackUrl(track.get(Metadata::Fields::ResourceUrl).toUrl()),
      m_trackNumber(track.get(Metadata::Fields::TrackNumber).toInt()),
      m_discNumber(track.get(Metadata::Fields::DiscNumber).toInt()),
      m_dbId(track.get(Metadata::Fields::DatabaseId).toULongLong()), m_isValid(true), m_entryType(PlayerUtils::Track) {}

PlaylistEntry::PlaylistEntry(QUrl fileName) : m_trackUrl(std::move(fileName)) {}

PlaylistEntry::PlaylistEntry(const quint64 id, const QString& entryTitle, const PlaylistModel::EntryType type)
    : m_title(entryTitle), m_dbId(id), m_isValid(true), m_entryType(type) {}
