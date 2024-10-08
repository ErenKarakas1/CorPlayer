#include <playlist/trackplaylist.h>

#include "playerutils.h"

#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QUrl>

#include <algorithm>

class TrackPlaylistPrivate {
public:
    QList<TrackPlaylistEntry> m_fields;
    QList<MetadataFields::TrackMetadataField> m_trackFields;
};

TrackPlaylist::TrackPlaylist(QObject *parent) : QAbstractListModel(parent), tp(new TrackPlaylistPrivate) {}

TrackPlaylist::~TrackPlaylist() = default;

int TrackPlaylist::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(tp->m_fields.size());
}

QHash<int, QByteArray> TrackPlaylist::roleNames() const {
    auto roles = QAbstractItemModel::roleNames();

    roles[static_cast<int>(ColumnRoles::IsValidRole)] = "isValid";
    roles[static_cast<int>(ColumnRoles::DatabaseIdRole)] = "databaseId";
    roles[static_cast<int>(ColumnRoles::TitleRole)] = "title";
    roles[static_cast<int>(ColumnRoles::StringDurationRole)] = "duration";
    roles[static_cast<int>(ColumnRoles::DurationRole)] = "durationInt";
    roles[static_cast<int>(ColumnRoles::ArtistRole)] = "artist";
    roles[static_cast<int>(ColumnRoles::AlbumArtistRole)] = "albumArtist";
    roles[static_cast<int>(ColumnRoles::AlbumRole)] = "album";
    roles[static_cast<int>(ColumnRoles::TrackNumberRole)] = "trackNumber";
    roles[static_cast<int>(ColumnRoles::DiscNumberRole)] = "discNumber";
    roles[static_cast<int>(ColumnRoles::GenreRole)] = "genre";
    roles[static_cast<int>(ColumnRoles::PerformerRole)] = "performer";
    roles[static_cast<int>(ColumnRoles::LyricistRole)] = "lyricist";
    roles[static_cast<int>(ColumnRoles::ComposerRole)] = "composer";
    roles[static_cast<int>(ColumnRoles::CommentRole)] = "comment";
    roles[static_cast<int>(ColumnRoles::YearRole)] = "year";
    roles[static_cast<int>(ColumnRoles::ChannelsRole)] = "channels";
    roles[static_cast<int>(ColumnRoles::BitRateRole)] = "bitRate";
    roles[static_cast<int>(ColumnRoles::SampleRateRole)] = "sampleRate";
    roles[static_cast<int>(ColumnRoles::CountRole)] = "count";
    roles[static_cast<int>(ColumnRoles::IsPlayingRole)] = "isPlaying";
    roles[static_cast<int>(ColumnRoles::IsSingleDiscAlbumRole)] = "isSingleDiscAlbum";
    roles[static_cast<int>(ColumnRoles::SecondaryTextRole)] = "secondaryText";
    roles[static_cast<int>(ColumnRoles::ImageUrlRole)] = "imageUrl";
    roles[static_cast<int>(ColumnRoles::ShadowForImageRole)] = "shadowForImage";
    roles[static_cast<int>(ColumnRoles::ResourceRole)] = "trackResource";
    roles[static_cast<int>(ColumnRoles::FullDataRole)] = "trackData";
    roles[static_cast<int>(ColumnRoles::AlbumIdRole)] = "albumId";
    roles[static_cast<int>(ColumnRoles::AlbumSectionRole)] = "albumSection";
    roles[static_cast<int>(ColumnRoles::ElementTypeRole)] = "entryType";
    roles[static_cast<int>(ColumnRoles::MetadataModifiableRole)] = "metadataModifiableRole";
    roles[static_cast<int>(ColumnRoles::HashRole)] = "hash";

    return roles;
}

QVariant TrackPlaylist::data(const QModelIndex &index, int role) const {
    auto result = QVariant();

    if (!index.isValid()) {
        return result;
    }

    if (tp->m_fields[index.row()].m_isValid) {
        switch (role) {
            case ColumnRoles::IsValidRole:
                result = tp->m_fields[index.row()].m_isValid;
                break;
            case ColumnRoles::IsPlayingRole:
                result = tp->m_fields[index.row()].m_isPlaying;
                break;
            case ColumnRoles::ElementTypeRole:
                result = QVariant::fromValue(tp->m_fields[index.row()].m_entryType);
                break;
            case ColumnRoles::DurationRole:
                result = tp->m_trackFields[index.row()].duration();
                break;
            case ColumnRoles::StringDurationRole: {
                const QTime trackDuration =
                    tp->m_trackFields[index.row()][TrackMetadataField::key_type::DurationRole].toTime();

                if (trackDuration.hour() == 0) {
                    result = trackDuration.toString(QStringLiteral("mm:ss"));
                } else {
                    result = trackDuration.toString();
                }
                break;
            }
            case ColumnRoles::AlbumSectionRole:
                result =
                    QJsonDocument{
                        QJsonArray{
                            tp->m_trackFields[index.row()][TrackMetadataField::key_type::AlbumRole].toString(),
                            tp->m_trackFields[index.row()][TrackMetadataField::key_type::AlbumArtistRole].toString(),
                            tp->m_trackFields[index.row()][TrackMetadataField::key_type::ImageUrlRole]
                                .toUrl()
                                .toString()}}
                        .toJson();
                break;
            case ColumnRoles::TitleRole: {
                const auto &trackData = tp->m_trackFields[index.row()];
                auto titleData = trackData[TrackMetadataField::key_type::TitleRole];

                if (titleData.toString().isEmpty()) {
                    result = trackData[TrackMetadataField::key_type::ResourceRole].toUrl().fileName();
                } else {
                    result = titleData;
                }
                break;
            }
            case ColumnRoles::MetadataModifiableRole:
                switch (tp->m_fields[index.row()].m_entryType) {
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
                        result = tp->m_trackFields[index.row()].resourceURI().isLocalFile();
                        break;
                }
                break;
            default:
                const auto &trackData = tp->m_trackFields[index.row()];
                auto roleEnum = static_cast<TrackMetadataField::key_type>(role);
                auto itData = trackData.find(roleEnum);
                if (itData != trackData.end()) {
                    result = itData.value();
                } else {
                    result = {};
                }
        }
    } else {
        switch (role) {
            case ColumnRoles::IsValidRole:
                result = tp->m_fields[index.row()].m_isValid;
                break;
            case ColumnRoles::TitleRole:
                result = tp->m_fields[index.row()].m_title;
                break;
            case ColumnRoles::IsPlayingRole:
                result = tp->m_fields[index.row()].m_isPlaying;
                break;
            case ColumnRoles::ArtistRole:
            case ColumnRoles::AlbumArtistRole:
                result = tp->m_fields[index.row()].m_artist;
                break;
            case ColumnRoles::AlbumRole:
                result = tp->m_fields[index.row()].m_album;
                break;
            case ColumnRoles::TrackNumberRole:
                result = -1;
                break;
            case ColumnRoles::IsSingleDiscAlbumRole:
                result = false;
                break;
            case Qt::DisplayRole:
                result = tp->m_fields[index.row()].m_title;
                break;
            case ColumnRoles::ImageUrlRole:
                result = QUrl(QStringLiteral("error image")); // TODO: change to error image
                break;
            case ColumnRoles::ShadowForImageRole:
                result = false;
                break;
            case ColumnRoles::AlbumSectionRole:
                result = QJsonDocument{QJsonArray{tp->m_fields[index.row()].m_album.toString(),
                                                  tp->m_fields[index.row()].m_artist.toString(),
                                                  QUrl(QStringLiteral("error image")).toString()}}
                             .toJson(); // TODO: change to error image
                break;
            default:
                result = {};
        }
    }
    return result;
}

bool TrackPlaylist::setData(const QModelIndex &index, const QVariant &value, int role) {
    bool modelModified = false;

    if (!index.isValid()) {
        return modelModified;
    }

    if (index.row() < 0 || index.row() >= tp->m_fields.size()) {
        return modelModified;
    }

    if (role < ColumnRoles::IsValidRole || role > ColumnRoles::IsPlayingRole) {
        return modelModified;
    }

    auto convertedRole = static_cast<ColumnRoles>(role);

    switch (convertedRole) {
        case ColumnRoles::IsPlayingRole: {
            modelModified = true;
            const auto newState = static_cast<PlayState>(value.toInt());
            tp->m_fields[index.row()].m_isPlaying = newState;
            Q_EMIT dataChanged(index, index, {role});

            break;
        }
        case ColumnRoles::TitleRole: {
            modelModified = true;
            tp->m_fields[index.row()].m_title = value;
            tp->m_trackFields[index.row()][static_cast<TrackMetadataField::key_type>(role)] = value;
            Q_EMIT dataChanged(index, index, {role});

            break;
        }
        case ColumnRoles::ArtistRole: {
            modelModified = true;
            tp->m_fields[index.row()].m_artist = value;
            tp->m_trackFields[index.row()][static_cast<TrackMetadataField::key_type>(role)] = value;
            Q_EMIT dataChanged(index, index, {role});

            break;
        }
        default:
            modelModified = false;
    }

    return modelModified;
}

bool TrackPlaylist::removeRows(const int row, const int count, const QModelIndex &parent) {
    beginRemoveRows(parent, row, row + count - 1);

    for (int i = row, cpt = 0; cpt < count; ++i, ++cpt) {
        tp->m_fields.removeAt(i);
        tp->m_trackFields.removeAt(i);
    }
    endRemoveRows();

    return true;
}

bool TrackPlaylist::moveRows(const QModelIndex &sourceParent, const int sourceRow, const int count,
                             const QModelIndex &destinationParent, const int destinationChild) {
    if (sourceParent != destinationParent) {
        return false;
    }

    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild)) {
        return false;
    }

    for (auto cptItem = 0; cptItem < count; ++cptItem) {
        if (sourceRow < destinationChild) {
            tp->m_fields.move(sourceRow, destinationChild - 1);
            tp->m_trackFields.move(sourceRow, destinationChild - 1);
        } else {
            tp->m_fields.move(sourceRow, destinationChild);
            tp->m_trackFields.move(sourceRow, destinationChild);
        }
    }

    endMoveRows();

    return true;
}

void TrackPlaylist::enqueueRestoredEntries(const QVariantList &newEntries) {
    if (newEntries.isEmpty()) {
        return;
    }

    beginInsertRows(QModelIndex(), tp->m_fields.size(), tp->m_fields.size() + newEntries.size() - 1);

    for (const auto &oneData : newEntries) {
        auto trackData = oneData.toStringList();
        if (trackData.size() != 7 && trackData.size() != 8) {
            continue;
        }

        auto restoredId = trackData[0].toULongLong();
        const auto &restoredTitle = trackData[1];
        const auto &restoredArtist = trackData[2];
        const auto &restoredAlbum = trackData[3];
        const auto &restoredTrackNumber = trackData[4];
        const auto &restoredDiscNumber = trackData[5];
        auto restoredFileUrl = QVariant{};

        if (trackData.size() == 8) {
            restoredFileUrl = QUrl{trackData[7]};
        }

        auto m_entryType = static_cast<PlayerUtils::PlaylistEntryType>(trackData[6].toInt());
        auto newEntry = TrackPlaylistEntry({restoredId, restoredTitle, restoredArtist, restoredAlbum, restoredFileUrl,
                                            restoredTrackNumber, restoredDiscNumber, m_entryType});

        tp->m_fields.push_back(newEntry);
        tp->m_trackFields.push_back({});

        if (newEntry.m_trackUrl.isValid()) {
            auto entryURL = newEntry.m_trackUrl.toUrl();
            if (entryURL.isLocalFile()) {
                auto entryString = entryURL.toLocalFile();
                const QFileInfo newTrackFile(entryString);
                if (newTrackFile.exists()) {
                    tp->m_fields.last().m_isValid = true;
                    Q_EMIT addNewEntry(0, entryString, PlayerUtils::FileName);
                } else if (newEntry.m_title.toString().isEmpty()) {
                    Q_EMIT addNewEntry(0, entryString, PlayerUtils::FileName);
                } else {
                    Q_EMIT addTrackByName(newEntry.m_title, newEntry.m_artist, newEntry.m_album, newEntry.m_trackNumber,
                                          newEntry.m_discNumber);
                }
            } else {
                tp->m_fields.last().m_isValid = true;
            }
        } else {
            Q_EMIT addTrackByName(newEntry.m_title, newEntry.m_artist, newEntry.m_album, newEntry.m_trackNumber,
                                  newEntry.m_discNumber);
        }
    }
    endInsertRows();
}

void TrackPlaylist::enqueueOneEntry(const MetadataFields::EntryMetadata &entryData, const int insertAt) {
    enqueueMultipleEntries({entryData}, insertAt);
}

void TrackPlaylist::enqueueMultipleEntries(const MetadataFields::EntryMetadataList &entriesData, const int insertAt) {
    const int validEntries =
        std::accumulate(entriesData.cbegin(), entriesData.cend(), 0, [](const int validEntries, const auto &entryData) {
            return entryData.isValid() ? validEntries + 1 : validEntries;
        });

    if (validEntries == 0) {
        return;
    }

    tp->m_fields.reserve(tp->m_fields.size() + validEntries);
    tp->m_trackFields.reserve(tp->m_fields.size() + validEntries);

    int i = insertAt < 0 || insertAt > tp->m_fields.size() ? tp->m_fields.size() : insertAt;
    beginInsertRows(QModelIndex(), i, i + validEntries - 1);

    for (const auto &entryData : entriesData) {
        if (!entryData.isValid()) {
            continue;
        }

        const auto trackUrl =
            entryData.url.isValid() ? entryData.url : entryData.musicMetadata[MetadataFields::ResourceRole].toUrl();

        if (!entryData.musicMetadata.hasDatabaseId() && trackUrl.isValid()) {
            auto newEntry = TrackPlaylistEntry{trackUrl};
            newEntry.m_entryType = PlayerUtils::FileName;

            tp->m_fields.insert(i, std::move(newEntry));
            tp->m_trackFields.insert(i, {});
        } else {
            tp->m_fields.insert(i, TrackPlaylistEntry{entryData.musicMetadata.databaseId(), entryData.title,
                                                      entryData.musicMetadata.elementType()});
            const auto &data = entryData.musicMetadata;

            switch (data.elementType()) {
                case PlayerUtils::Track:
                case PlayerUtils::FileName:
                    tp->m_trackFields.insert(i, static_cast<const MetadataFields::TrackMetadataField &>(data));
                    break;
                default:
                    tp->m_trackFields.insert(i, {});
            }
        }

        if (trackUrl.isValid()) {
            Q_EMIT addNewUrl(trackUrl, entryData.musicMetadata.hasElementType() ? entryData.musicMetadata.elementType()
                                                                                : PlayerUtils::FileName);

        } else {
            Q_EMIT addNewEntry(entryData.musicMetadata.databaseId(), entryData.title,
                               entryData.musicMetadata.elementType());
        }
        ++i;
    }
    endInsertRows();
}

void TrackPlaylist::clearPlaylist() {
    if (tp->m_fields.isEmpty()) return;

    beginRemoveRows({}, 0, static_cast<int>(tp->m_fields.count()) - 1);

    tp->m_fields.clear();
    tp->m_trackFields.clear();

    endRemoveRows();
}

QVariantList TrackPlaylist::getEntriesForRestore() const {
    QVariantList result;

    for (int trackIndex = 0; trackIndex < tp->m_fields.size(); ++trackIndex) {
        const auto &oneEntry = tp->m_fields[trackIndex];
        if (oneEntry.m_isValid) {
            QStringList oneData;
            const auto &oneTrack = tp->m_trackFields[trackIndex];

            oneData.push_back(QString::number(oneTrack.databaseId()));
            oneData.push_back(oneTrack.title());
            oneData.push_back(oneTrack.artist());

            if (oneTrack.hasRole(MetadataFields::AlbumRole)) {
                oneData.push_back(oneTrack.album());
            } else {
                oneData.push_back({});
            }

            if (oneTrack.hasRole(MetadataFields::TrackNumberRole)) {
                oneData.push_back(QString::number(oneTrack.trackNumber()));
            } else {
                oneData.push_back({});
            }

            if (oneTrack.hasRole(MetadataFields::DiscNumberRole)) {
                oneData.push_back(QString::number(oneTrack.discNumber()));
            } else {
                oneData.push_back({});
            }
            oneData.push_back(QString::number(oneEntry.m_entryType));
            oneData.push_back(oneTrack.resourceURI().toString());

            result.push_back(QVariant(oneData));
        }
    }
    return result;
}

void TrackPlaylist::tracksListAdded(const qulonglong newDatabaseId, const QString &entryTitle,
                                    const PlayerUtils::PlaylistEntryType databaseIdType,
                                    const ListTrackMetadataField &tracks) {
    if (tracks.isEmpty()) {
        return;
    }

    for (int playListIndex = 0; playListIndex < tp->m_fields.size(); ++playListIndex) {
        auto &oneEntry = tp->m_fields[playListIndex];
        if (oneEntry.m_entryType != databaseIdType) {
            continue;
        }

        if (oneEntry.m_title != entryTitle) {
            continue;
        }

        if (newDatabaseId != 0 && oneEntry.m_id != newDatabaseId) {
            continue;
        }

        beginRemoveRows(QModelIndex(), playListIndex, playListIndex);
        tp->m_fields.removeAt(playListIndex);
        tp->m_trackFields.removeAt(playListIndex);
        endRemoveRows();

        beginInsertRows(QModelIndex(), playListIndex, playListIndex - 1 + static_cast<int>(tracks.size()));

        for (int trackIndex = 0; trackIndex < tracks.size(); ++trackIndex) {
            auto newEntry = TrackPlaylistEntry{tracks[trackIndex]};
            newEntry.m_entryType = PlayerUtils::Track;
            tp->m_fields.insert(playListIndex + trackIndex, newEntry);
            tp->m_trackFields.insert(playListIndex + trackIndex, tracks[trackIndex]);
        }

        endInsertRows();
    }
}

void TrackPlaylist::trackChanged(const TrackMetadataField &track) {
    for (int i = 0; i < tp->m_fields.size(); ++i) {
        auto &oneEntry = tp->m_fields[i];

        if (oneEntry.m_entryType != PlayerUtils::Artist && oneEntry.m_isValid) {
            if (oneEntry.m_trackUrl.toUrl().isValid() && track.resourceURI() != oneEntry.m_trackUrl.toUrl()) {
                continue;
            }

            if (!oneEntry.m_trackUrl.toUrl().isValid() && (oneEntry.m_id == 0 || track.databaseId() != oneEntry.m_id)) {
                continue;
            }

            const auto &trackData = tp->m_trackFields[i];

            if (!trackData.empty()) {
                bool sameData = true;
                for (auto oneKeyIterator = track.constKeyValueBegin(); oneKeyIterator != track.constKeyValueEnd();
                     ++oneKeyIterator) {
                    if (trackData[oneKeyIterator->first] != oneKeyIterator->second) {
                        sameData = false;
                        break;
                    }
                }
                if (sameData) {
                    continue;
                }
            }

            tp->m_trackFields[i] = track;

            Q_EMIT dataChanged(index(i, 0), index(i, 0), {});
            continue;
        }

        if (oneEntry.m_entryType != PlayerUtils::Artist && !oneEntry.m_isValid && !oneEntry.m_trackUrl.isValid()) {
            if (track.find(TrackMetadataField::key_type::TitleRole) != track.end() &&
                track.title() != oneEntry.m_title) {
                continue;
            }

            if (track.find(TrackMetadataField::key_type::AlbumRole) != track.end() &&
                track.album() != oneEntry.m_album) {
                continue;
            }

            if (track.find(TrackMetadataField::key_type::TrackNumberRole) != track.end() &&
                track.trackNumber() != oneEntry.m_trackNumber) {
                continue;
            }

            if (track.find(TrackMetadataField::key_type::DiscNumberRole) != track.end() &&
                track.discNumber() != oneEntry.m_discNumber) {
                continue;
            }

            tp->m_trackFields[i] = track;
            oneEntry.m_id = track.databaseId();
            oneEntry.m_isValid = true;

            Q_EMIT dataChanged(index(i, 0), index(i, 0), {});

            break;
        }

        if (oneEntry.m_entryType != PlayerUtils::Artist && !oneEntry.m_isValid && oneEntry.m_trackUrl.isValid()) {
            if (track.resourceURI() != oneEntry.m_trackUrl) {
                continue;
            }

            tp->m_trackFields[i] = track;
            oneEntry.m_id = track.databaseId();
            oneEntry.m_isValid = true;

            Q_EMIT dataChanged(index(i, 0), index(i, 0), {});
            break;
        }
    }
}

void TrackPlaylist::trackRemoved(const qulonglong trackId) {
    for (int i = 0; i < tp->m_fields.size(); ++i) {
        auto &oneEntry = tp->m_fields[i];

        if (oneEntry.m_isValid) {
            if (oneEntry.m_id == trackId) {
                oneEntry.m_isValid = false;
                oneEntry.m_title = tp->m_trackFields[i].title();
                oneEntry.m_artist = tp->m_trackFields[i].artist();
                oneEntry.m_album = tp->m_trackFields[i].album();
                oneEntry.m_trackNumber = tp->m_trackFields[i].trackNumber();
                oneEntry.m_discNumber = tp->m_trackFields[i].discNumber();

                Q_EMIT dataChanged(index(i, 0), index(i, 0), {});
            }
        }
    }
}

void TrackPlaylist::trackInError(const QUrl &sourceInError, const QMediaPlayer::Error playerError) {
    Q_UNUSED(playerError)

    for (int i = 0; i < tp->m_fields.size(); ++i) {
        auto &oneTrack = tp->m_fields[i];
        if (oneTrack.m_isValid) {
            const auto &oneTrackData = tp->m_trackFields.at(i);

            if (oneTrackData.resourceURI() == sourceInError) {
                oneTrack.m_isValid = false;
                Q_EMIT dataChanged(index(i, 0), index(i, 0), {ColumnRoles::IsValidRole});
            }
        }
    }
}

QDebug operator<<(const QDebug &stream, const TrackPlaylistEntry &data) {
    stream << data.m_title << data.m_album << data.m_artist << data.m_trackUrl << data.m_trackNumber
           << data.m_discNumber << data.m_id << data.m_isValid;
    return stream;
}
