#include "models/playlistproxymodel.hpp"

#include "library/library.hpp"
#include "models/playlistmodel.hpp"

#include "playerutils.hpp"

#include <QItemSelection>
#include <QRandomGenerator>
#include <QTime>

#include <algorithm>

class PlaylistProxyModelPrivate {
public:
    Library* m_library = nullptr;
    PlaylistModel* m_playlistModel = nullptr;
    QPersistentModelIndex m_previousTrack;
    QPersistentModelIndex m_currentTrack;
    QPersistentModelIndex m_nextTrack;

    bool m_currentTrackWasValid = false;
    QList<int> m_randomMapping;
    QVariantMap m_persistentSettingsForUndo;
    QRandomGenerator m_randomGenerator{static_cast<quint32>(QTime::currentTime().msecsSinceStartOfDay())};
    PlayerUtils::PlaylistEnqueueTriggerPlay m_triggerPlay = PlayerUtils::DoNotTriggerPlay;
    int m_currentPlaylistPosition = -1;

    PlaylistProxyModel::Repeat m_repeatMode = PlaylistProxyModel::Repeat::None;
    PlaylistProxyModel::Shuffle m_shuffleMode = PlaylistProxyModel::Shuffle::NoShuffle;
};

PlaylistProxyModel::PlaylistProxyModel(Library* library, QObject* parent)
    : QAbstractProxyModel(parent), pp(std::make_unique<PlaylistProxyModelPrivate>()) {
    pp->m_library = library;
}

PlaylistProxyModel::~PlaylistProxyModel() = default;

QModelIndex PlaylistProxyModel::index(const int row, const int column, const QModelIndex& parent) const {
    Q_UNUSED(parent);
    if (row < 0 || column < 0 || row >= rowCount()) return {};
    return createIndex(row, column);
}

QModelIndex PlaylistProxyModel::mapFromSource(const QModelIndex& sourceIndex) const {
    if (!sourceIndex.isValid()) return {};
    return pp->m_playlistModel->index(mapRowFromSource(sourceIndex.row()), sourceIndex.column());
}

QItemSelection PlaylistProxyModel::mapSelectionFromSource(const QItemSelection& sourceSelection) const {
    QItemSelection proxySelection;
    for (const QItemSelectionRange& range : sourceSelection) {
        const QModelIndex proxyTopLeft = mapFromSource(range.topLeft());
        const QModelIndex proxyBottomRight = mapFromSource(range.bottomRight());
        proxySelection.append(QItemSelectionRange(proxyTopLeft, proxyBottomRight));
    }

    return proxySelection;
}

QItemSelection PlaylistProxyModel::mapSelectionToSource(const QItemSelection& proxySelection) const {
    QItemSelection sourceSelection;
    for (const QItemSelectionRange& range : proxySelection) {
        const QModelIndex sourceTopLeft = mapToSource(range.topLeft());
        const QModelIndex sourceBottomRight = mapToSource(range.bottomRight());
        sourceSelection.append(QItemSelectionRange(sourceTopLeft, sourceBottomRight));
    }

    return sourceSelection;
}

QModelIndex PlaylistProxyModel::mapToSource(const QModelIndex& proxyIndex) const {
    if (!proxyIndex.isValid()) return {};
    return pp->m_playlistModel->index(mapRowToSource(proxyIndex.row()), proxyIndex.column());
}

int PlaylistProxyModel::mapRowToSource(const int proxyRow) const {
    if (pp->m_shuffleMode != NoShuffle) {
        return pp->m_randomMapping.at(proxyRow);
    }
    return proxyRow;
}

int PlaylistProxyModel::mapRowFromSource(const int sourceRow) const {
    if (pp->m_shuffleMode != NoShuffle) {
        return static_cast<int>(pp->m_randomMapping.indexOf(sourceRow));
    }
    return sourceRow;
}

int PlaylistProxyModel::rowCount(const QModelIndex& parent) const {
    if (pp->m_shuffleMode != NoShuffle) {
        if (parent.isValid()) return 0;
        return static_cast<int>(pp->m_randomMapping.count());
    }
    return pp->m_playlistModel != nullptr ? pp->m_playlistModel->rowCount(parent) : 0;
}

int PlaylistProxyModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 1;
}

QModelIndex PlaylistProxyModel::parent(const QModelIndex& child) const {
    Q_UNUSED(child);
    return {};
}

bool PlaylistProxyModel::hasChildren(const QModelIndex& parent) const {
    return !parent.isValid() && rowCount() > 0;
}

void PlaylistProxyModel::setPlaylistModel(PlaylistModel* playlistModel) {
    if (pp->m_playlistModel != nullptr) {
        disconnect(pp->m_playlistModel, nullptr, this, nullptr);
    }

    pp->m_playlistModel = playlistModel;
    setSourceModel(playlistModel);

    // clang-format off
    if (pp->m_playlistModel != nullptr) {
        connect(playlistModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &PlaylistProxyModel::sourceRowsAboutToBeInserted);
        connect(playlistModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &PlaylistProxyModel::sourceRowsAboutToBeRemoved);
        connect(playlistModel, &QAbstractItemModel::rowsAboutToBeMoved, this, &PlaylistProxyModel::sourceRowsAboutToBeMoved);
        connect(playlistModel, &QAbstractItemModel::rowsInserted, this, &PlaylistProxyModel::sourceRowsInserted);
        connect(playlistModel, &QAbstractItemModel::rowsRemoved, this, &PlaylistProxyModel::sourceRowsRemoved);
        connect(playlistModel, &QAbstractItemModel::rowsMoved, this, &PlaylistProxyModel::sourceRowsMoved);
        connect(playlistModel, &QAbstractItemModel::dataChanged, this, &PlaylistProxyModel::sourceDataChanged);
        connect(playlistModel, &QAbstractItemModel::layoutAboutToBeChanged, this, &PlaylistProxyModel::sourceLayoutAboutToBeChanged);
        connect(playlistModel, &QAbstractItemModel::layoutChanged, this, &PlaylistProxyModel::sourceLayoutChanged);
        connect(playlistModel, &QAbstractItemModel::modelAboutToBeReset, this, &PlaylistProxyModel::sourceModelAboutToBeReset);
        connect(playlistModel, &QAbstractItemModel::modelReset, this, &PlaylistProxyModel::sourceModelReset);
    }
    // clang-format on
}

QPersistentModelIndex PlaylistProxyModel::previousTrack() const {
    return pp->m_previousTrack;
}

QPersistentModelIndex PlaylistProxyModel::currentTrack() const {
    return pp->m_currentTrack;
}

QPersistentModelIndex PlaylistProxyModel::nextTrack() const {
    return pp->m_nextTrack;
}

PlaylistProxyModel::Repeat PlaylistProxyModel::repeatMode() const {
    return pp->m_repeatMode;
}

PlaylistProxyModel::Shuffle PlaylistProxyModel::shuffleMode() const {
    return pp->m_shuffleMode;
}

int PlaylistProxyModel::totalTracksDuration() const {
    int duration = 0;
    for (int i = 0; i < rowCount(); ++i) {
        duration += pp->m_playlistModel->data(index(i, 0), Metadata::Fields::Duration).toTime().msecsSinceStartOfDay();
    }
    return duration;
}

int PlaylistProxyModel::remainingTracksDuration() const {
    int duration = 0;
    for (int i = pp->m_currentTrack.row(); i < rowCount(); ++i) {
        duration += pp->m_playlistModel->data(index(i, 0), Metadata::Fields::Duration).toTime().msecsSinceStartOfDay();
    }
    return duration;
}

int PlaylistProxyModel::remainingTracks() const {
    if (!pp->m_currentTrack.isValid() || pp->m_repeatMode == CurrentTrack || pp->m_repeatMode == Playlist) {
        return -1;
    }
    return rowCount() - pp->m_currentTrack.row() - 1;
}

int PlaylistProxyModel::currentTrackRow() const {
    return pp->m_currentTrack.row();
}

int PlaylistProxyModel::tracksCount() const {
    return rowCount();
}

QVariantMap PlaylistProxyModel::persistentState() const {
    QVariantMap currentState;

    currentState[QStringLiteral("playlist")] = pp->m_playlistModel->getEntriesForRestore();
    currentState[QStringLiteral("shuffleMode")] = pp->m_shuffleMode;
    currentState[QStringLiteral("randomMapping")] = getRandomMappingForRestore();
    currentState[QStringLiteral("currentTrack")] = pp->m_currentPlaylistPosition;
    currentState[QStringLiteral("repeatMode")] = pp->m_repeatMode;

    return currentState;
}

void PlaylistProxyModel::enqueue(const QUrl& entryUrl, const PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                 const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    auto entry = Metadata::TrackFields();
    entry.insert(Metadata::Fields::ElementType, PlayerUtils::Track);
    enqueue({{.trackFields = entry, .title = {}, .url = entryUrl}}, enqueueMode, triggerPlay);
}

void PlaylistProxyModel::enqueue(const Metadata::TrackFields& newEntry, const QString& newEntryTitle,
                                 const PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                 const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    enqueue({{.trackFields = newEntry, .title = newEntryTitle, .url = {}}}, enqueueMode, triggerPlay);
}

void PlaylistProxyModel::enqueue(const Metadata::EntryFieldsList& newEntries,
                                 const PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                 const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    if (newEntries.isEmpty()) return;

    pp->m_triggerPlay = triggerPlay;

    if (enqueueMode == PlayerUtils::ReplacePlaylist) {
        if (rowCount() != 0) {
            clearPlaylist();
        }
        pp->m_currentPlaylistPosition = -1;
    }
    const int enqueueIndex =
        enqueueMode == PlayerUtils::AfterCurrentTrack ? mapRowToSource(pp->m_currentTrack.row()) + 1 : -1;

    pp->m_playlistModel->enqueueMultipleEntries(newEntries, enqueueIndex);

    if (enqueueMode == PlayerUtils::ReplacePlaylist) {
        pp->m_currentPlaylistPosition = 0;
        determineTracks();
    }
}

void PlaylistProxyModel::loadPlaylistFromDatabase(const quint64 playlistId) {
    const auto tracks = pp->m_library->playlistDatabase().getPlaylistTracks(playlistId);

    Metadata::EntryFieldsList entries;
    entries.reserve(tracks.size());

    for (const quint64& trackId : tracks) {
        auto track = pp->m_library->getTrackById(trackId);
        if (track.isValid()) {
            entries.emplace_back(Metadata::EntryFields{.trackFields = track, .title = {}, .url = {}});
        }
    }

    enqueue(entries, PlayerUtils::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    Q_EMIT playlistLoaded();
}

void PlaylistProxyModel::loadPlaylistFromFile(const QUrl& fileName) {
    if (pp->m_playlistModel == nullptr) return;

    clearPlaylist();
    const auto playlistId = pp->m_library->importPlaylist(fileName);
    if (playlistId == 0) {
        Q_EMIT playlistLoadFailed();
        return;
    }

    loadPlaylistFromDatabase(playlistId);

    Q_EMIT playlistImported(playlistId);
}

void PlaylistProxyModel::clearPlaylist() {
    if (rowCount() == 0) return;

    pp->m_persistentSettingsForUndo = persistentState();
    pp->m_currentPlaylistPosition = -1;
    pp->m_currentTrack = QPersistentModelIndex{};
    pp->m_previousTrack = QPersistentModelIndex{};
    pp->m_nextTrack = QPersistentModelIndex{};

    notifyCurrentTrackChanged();

    pp->m_playlistModel->clearPlaylist();

    Q_EMIT tracksCountChanged();
    Q_EMIT totalTracksDurationChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
    Q_EMIT persistentStateChanged();
}

void PlaylistProxyModel::skipNextTrack(const PlayerUtils::SkipReason reason) {
    if (!pp->m_currentTrack.isValid()) return;
    if (pp->m_repeatMode == Repeat::None && pp->m_currentTrack.row() == rowCount() - 1) {
        Q_EMIT playlistFinished();
        return;
    }

    // TODO: fixup
    if (pp->m_repeatMode == Repeat::CurrentTrack && reason == PlayerUtils::SkipReason::Automatic) {
        pp->m_currentTrack = index(pp->m_currentTrack.row(), 0);
    } else {
        const int nextRow = pp->m_currentTrack.row() + 1;
        if (nextRow < rowCount()) {
            pp->m_currentTrack = index(nextRow, 0);
        } else {
            if (pp->m_repeatMode == Repeat::None) {
                pp->m_currentTrack = QPersistentModelIndex();
                Q_EMIT playlistFinished();
            } else {
                pp->m_currentTrack = index(0, 0);
            }
        }
    }

    pp->m_currentPlaylistPosition = pp->m_currentTrack.row();
    notifyCurrentTrackChanged();
}

void PlaylistProxyModel::skipPreviousTrack(const qint64 position) {
    if (!pp->m_currentTrack.isValid()) return;

    if (position > m_seekToBeginningDelay) {
        Q_EMIT seek(0);
        return;
    }

    if (pp->m_currentTrack.row() == 0) {
        if (pp->m_repeatMode == Repeat::CurrentTrack || pp->m_repeatMode == Repeat::Playlist) {
            pp->m_currentTrack = index(rowCount() - 1, 0);
        } else {
            return;
        }
    } else {
        pp->m_currentTrack = index(pp->m_currentTrack.row() - 1, 0);
    }

    notifyCurrentTrackChanged();
}

void PlaylistProxyModel::switchTo(const int row) {
    if (!pp->m_currentTrack.isValid()) return;

    pp->m_currentTrack = index(row, 0);
    notifyCurrentTrackChanged();
}

void PlaylistProxyModel::removeSelection(const QList<int>& selection) {
    auto sortedSelection = selection;
    std::ranges::sort(sortedSelection, std::greater());

    for (const auto row : sortedSelection) {
        pp->m_playlistModel->removeRow(mapRowToSource(row));
    }
}

void PlaylistProxyModel::moveRow(const int from, const int to) {
    const bool currentTrackIndexChanged = (from < to)
                                            ? (from <= pp->m_currentTrack.row() && pp->m_currentTrack.row() <= to)
                                            : (to <= pp->m_currentTrack.row() && pp->m_currentTrack.row() <= from);

    if (pp->m_shuffleMode != NoShuffle) {
        beginMoveRows({}, from, from, {}, from < to ? to + 1 : to);
        pp->m_randomMapping.move(from, to);
        endMoveRows();
    } else {
        pp->m_playlistModel->moveRows({}, from, 1, {}, from < to ? to + 1 : to);
    }

    if (currentTrackIndexChanged) {
        notifyCurrentTrackRowChanged();
    }
}

void PlaylistProxyModel::setRepeatMode(const Repeat newMode) {
    if (pp->m_repeatMode != newMode) {
        pp->m_repeatMode = newMode;

        Q_EMIT repeatModeChanged();
        Q_EMIT remainingTracksChanged();
        Q_EMIT remainingTracksDurationChanged();
        Q_EMIT persistentStateChanged();

        determineAndNotifyPreviousAndNextTracks();
    }
}

void PlaylistProxyModel::setShuffleMode(const Shuffle value) {
    if (pp->m_shuffleMode == value) return;

    const int playlistSize = pp->m_playlistModel->rowCount();

    if (playlistSize != 0) {
        Q_EMIT layoutAboutToBeChanged({}, QAbstractItemModel::VerticalSortHint);

        if (!pp->m_randomMapping.isEmpty()) {
            Q_ASSERT(pp->m_shuffleMode != PlaylistProxyModel::Shuffle::NoShuffle);
            QModelIndexList from;
            QModelIndexList to;
            from.reserve(playlistSize);
            to.reserve(playlistSize);

            for (int i = 0; i < playlistSize; ++i) {
                to.append(index(pp->m_randomMapping.at(i), 0));
                from.append(index(i, 0));
            }

            changePersistentIndexList(from, to);
            pp->m_randomMapping.clear();
        }

        if (value != Shuffle::NoShuffle) {
            QModelIndexList from;
            QModelIndexList to;
            from.reserve(playlistSize);
            to.reserve(playlistSize);

            pp->m_randomMapping.clear();
            pp->m_randomMapping.reserve(playlistSize);

            for (int i = 0; i < playlistSize; ++i) {
                pp->m_randomMapping.append(i);
                to.append(index(i, 0));
            }

            if (value == Shuffle::Track) {
                if (playlistSize > 1) {
                    if (currentTrackRow() != 0) {
                        std::swap(pp->m_randomMapping[0], pp->m_randomMapping[currentTrackRow()]);
                    }
                    from.append(index(pp->m_randomMapping.at(0), 0));
                }

                for (int i = 1; i < playlistSize - 1; ++i) {
                    const int swapIndex = pp->m_randomGenerator.bounded(i, playlistSize);
                    std::swap(pp->m_randomMapping[i], pp->m_randomMapping[swapIndex]);
                    from.append(index(pp->m_randomMapping.at(i), 0));
                }
                from.append(index(pp->m_randomMapping.at(playlistSize - 1), 0));
            }
            changePersistentIndexList(from, to);
        }

        pp->m_currentPlaylistPosition = pp->m_currentTrack.row();
        pp->m_shuffleMode = value;

        Q_EMIT layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
        determineAndNotifyPreviousAndNextTracks();
    } else {
        pp->m_shuffleMode = value;
    }

    Q_EMIT shuffleModeChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
    Q_EMIT persistentStateChanged();
}

void PlaylistProxyModel::setPersistentState(const QVariantMap& persistentState) {
    auto playlistIt = persistentState.find(QStringLiteral("playlist"));
    if (playlistIt != persistentState.end()) {
        pp->m_playlistModel->enqueueRestoredEntries(playlistIt->toList());
    }

    const auto shuffleModeIt = persistentState.find(QStringLiteral("shuffleMode"));
    const auto shuffleMappingIt = persistentState.find(QStringLiteral("randomMapping"));
    if (shuffleModeIt != persistentState.end() && shuffleMappingIt != persistentState.end()) {
        restoreShuffleMode(shuffleModeIt->value<Shuffle>(), shuffleMappingIt->toList());
    }

    auto currTrackIt = persistentState.find(QStringLiteral("currentTrack"));
    if (currTrackIt != persistentState.end()) {
        auto newIndex = index(currTrackIt->toInt(), 0);
        if (newIndex.isValid() && (newIndex != pp->m_currentTrack)) {
            pp->m_currentTrack = newIndex;
            notifyCurrentTrackChanged();
        }
    }

    auto repeatModeIt = persistentState.find(QStringLiteral("repeatMode"));
    if (repeatModeIt != persistentState.end()) {
        setRepeatMode(repeatModeIt->value<Repeat>());
    }

    Q_EMIT persistentStateChanged();
}

void PlaylistProxyModel::trackInError(const QUrl& sourceInError, const QMediaPlayer::Error error) {
    Q_UNUSED(error)
    for (int i = 0; i < rowCount(); ++i) {
        const QUrl trackUrl = data(index(i, 0), PlaylistModel::ResourceUrlRole).toUrl();
        if (trackUrl == sourceInError) {
            setData(index(i, 0), false, PlaylistModel::IsValidRole);
        }
    }
}

void PlaylistProxyModel::sourceRowsAboutToBeInserted(const QModelIndex& parent, const int start, const int end) {
    if (pp->m_shuffleMode == NoShuffle) {
        beginInsertRows(parent, start, end);
    }
}

void PlaylistProxyModel::sourceRowsInserted(const QModelIndex& parent, const int start, const int end) {
    if (pp->m_shuffleMode == Track) {
        const auto newItemsCount = end - start + 1;
        pp->m_randomMapping.reserve(rowCount() + newItemsCount);

        if (rowCount() == 0) {
            beginInsertRows(parent, start, end);

            for (int i = 0; i < newItemsCount; ++i) {
                const auto random = pp->m_randomGenerator.bounded(pp->m_randomMapping.count() + 1);
                pp->m_randomMapping.insert(random, start + i);
            }
            endInsertRows();
        } else {
            if (start <= rowCount()) {
                std::ranges::transform(pp->m_randomMapping, pp->m_randomMapping.begin(), [=](const int sourceRow) {
                    return sourceRow < start ? sourceRow : sourceRow + newItemsCount;
                });
            }

            const bool enqueueAfterCurrentTrack = mapRowFromSource(start - 1) == pp->m_currentTrack.row();

            if (enqueueAfterCurrentTrack) {
                // Internally shuffle the new tracks then enqueue them all immediately after the current track
                QList<int> shuffledSourceRows(newItemsCount);
                std::iota(shuffledSourceRows.begin(), shuffledSourceRows.end(), start);
                std::ranges::shuffle(shuffledSourceRows, pp->m_randomGenerator);

                const int proxyStart = pp->m_currentTrack.row() + 1;
                beginInsertRows(parent, proxyStart, proxyStart + newItemsCount - 1);
                for (const int sourceRow : shuffledSourceRows) {
                    pp->m_randomMapping.insert(proxyStart, sourceRow);
                }
                endInsertRows();
            } else {
                const int lowerBound = pp->m_currentTrack.row() + 1;
                for (int i = 0; i < newItemsCount; ++i) {
                    const auto random = pp->m_randomGenerator.bounded(lowerBound, rowCount() + 1);
                    beginInsertRows(parent, random, random);
                    pp->m_randomMapping.insert(random, start + i);
                    endInsertRows();
                }
            }
        }
    } else {
        endInsertRows();
    }

    if (pp->m_currentTrack.isValid()) {
        pp->m_currentPlaylistPosition = pp->m_currentTrack.row();
        determineAndNotifyPreviousAndNextTracks();
    } else {
        determineTracks();
    }

    Q_EMIT tracksCountChanged();
    Q_EMIT totalTracksDurationChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
    Q_EMIT persistentStateChanged();
}

void PlaylistProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex& parent, const int start, const int end) {
    if (pp->m_shuffleMode != NoShuffle) {
        if (end - start + 1 == rowCount()) {
            beginRemoveRows(parent, start, end);
            pp->m_randomMapping.clear();
            endRemoveRows();
        }
        int row = 0;
        auto it = pp->m_randomMapping.begin();
        while (it != pp->m_randomMapping.end()) {
            if (*it >= start && *it <= end) {
                beginRemoveRows(parent, row, row);
                it = pp->m_randomMapping.erase(it);
                endRemoveRows();
            } else {
                if (*it > end) {
                    *it = *it - end + start - 1;
                }
                ++it;
                ++row;
            }
        }
    } else {
        pp->m_currentTrackWasValid = pp->m_currentTrack.isValid();
        beginRemoveRows(parent, start, end);
    }
}

void PlaylistProxyModel::sourceRowsRemoved(const QModelIndex& parent, const int start, const int end) {
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    if (pp->m_shuffleMode == NoShuffle) {
        endRemoveRows();
    }

    if (pp->m_currentTrack.isValid()) {
        pp->m_currentPlaylistPosition = pp->m_currentTrack.row();
    } else {
        pp->m_currentTrack = index(pp->m_currentPlaylistPosition, 0);

        if (pp->m_currentTrack.isValid() && pp->m_currentTrackWasValid) {
            notifyCurrentTrackChanged();
        } else {
            if (!pp->m_currentTrack.isValid()) {
                Q_EMIT playlistFinished();
                determineTracks();
                if (!pp->m_currentTrack.isValid() && pp->m_currentTrackWasValid) {
                    notifyCurrentTrackChanged();
                }
            }
        }
    }

    if (!pp->m_nextTrack.isValid() || !pp->m_previousTrack.isValid()) {
        determineAndNotifyPreviousAndNextTracks();
    }

    Q_EMIT tracksCountChanged();
    Q_EMIT totalTracksDurationChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
    Q_EMIT persistentStateChanged();
}

void PlaylistProxyModel::sourceRowsAboutToBeMoved(const QModelIndex& parent, const int start, const int end,
                                                  const QModelIndex& destParent, const int destRow) {
    Q_ASSERT(pp->m_shuffleMode == PlaylistProxyModel::Shuffle::NoShuffle);
    beginMoveRows(parent, start, end, destParent, destRow);
}

void PlaylistProxyModel::sourceRowsMoved(const QModelIndex& parent, const int start, const int end,
                                         const QModelIndex& destParent, const int destRow) {
    Q_ASSERT(pp->m_shuffleMode == PlaylistProxyModel::Shuffle::NoShuffle);

    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(destParent);
    Q_UNUSED(destRow);

    endMoveRows();

    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
    Q_EMIT persistentStateChanged();
}

void PlaylistProxyModel::sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                           const QList<int>& roles) {
    const auto startSourceRow = topLeft.row();
    const auto endSourceRow = bottomRight.row();

    for (int i = startSourceRow; i <= endSourceRow; ++i) {
        Q_EMIT dataChanged(index(mapRowFromSource(i), 0), index(mapRowFromSource(i), 0), roles);
        Q_EMIT remainingTracksDurationChanged();
        Q_EMIT totalTracksDurationChanged();

        if (i == pp->m_currentTrack.row()) {
            Q_EMIT currentTrackDataChanged();
        } else if (i == pp->m_previousTrack.row()) {
            Q_EMIT previousTrackDataChanged();
        } else if (i == pp->m_nextTrack.row()) {
            Q_EMIT nextTrackDataChanged();
        }
        determineTracks();
    }
}

void PlaylistProxyModel::sourceLayoutAboutToBeChanged() {
    Q_EMIT layoutAboutToBeChanged();
}

void PlaylistProxyModel::sourceLayoutChanged() {
    Q_EMIT layoutChanged();
}

void PlaylistProxyModel::sourceModelAboutToBeReset() {
    beginResetModel();
}

void PlaylistProxyModel::sourceModelReset() {
    endResetModel();
}

void PlaylistProxyModel::setSourceModel(QAbstractItemModel* sourceModel) {
    QAbstractProxyModel::setSourceModel(sourceModel);
    Q_EMIT tracksCountChanged();
    Q_EMIT totalTracksDurationChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
}

void PlaylistProxyModel::determineTracks() {
    if (!pp->m_currentTrack.isValid() || pp->m_currentPlaylistPosition != pp->m_currentTrack.row()) {
        for (int row = 0; row < rowCount(); ++row) {
            auto candidateTrack = index(row, 0);
            const auto type =
                candidateTrack.data(Metadata::Fields::ElementType).value<PlayerUtils::PlaylistEntryType>();

            if (candidateTrack.isValid() && candidateTrack.data(Metadata::Fields::IsValid).toBool() &&
                (type == PlayerUtils::Track || type == PlayerUtils::FileName)) {
                pp->m_currentTrack = candidateTrack;
                pp->m_currentPlaylistPosition = row;
                break;
            }
        }
        if (pp->m_currentTrack.isValid()) {
            notifyCurrentTrackChanged();

            if (pp->m_triggerPlay == PlayerUtils::TriggerPlay) {
                pp->m_triggerPlay = PlayerUtils::DoNotTriggerPlay;
                Q_EMIT ensurePlay();
            }
        }
    }
    if (!pp->m_nextTrack.isValid() || !pp->m_previousTrack.isValid()) {
        determineAndNotifyPreviousAndNextTracks();
    }
}

void PlaylistProxyModel::notifyCurrentTrackRowChanged() {
    if (pp->m_currentTrack.isValid()) {
        pp->m_currentPlaylistPosition = pp->m_currentTrack.row();
    } else {
        pp->m_currentPlaylistPosition = -1;
    }

    determineAndNotifyPreviousAndNextTracks();

    Q_EMIT currentTrackRowChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
}

void PlaylistProxyModel::notifyCurrentTrackChanged() {
    notifyCurrentTrackRowChanged();
    Q_EMIT currentTrackChanged(pp->m_currentTrack);
}

void PlaylistProxyModel::determineAndNotifyPreviousAndNextTracks() {
    if (!pp->m_currentTrack.isValid()) {
        pp->m_previousTrack = QPersistentModelIndex();
        pp->m_nextTrack = QPersistentModelIndex();
    }

    const auto oldPreviousTrack = pp->m_previousTrack;
    const auto oldNextTrack = pp->m_nextTrack;

    switch (pp->m_repeatMode) {
    case None:
        // return nothing if no tracks available
        if (pp->m_currentTrack.row() == 0) {
            pp->m_previousTrack = QPersistentModelIndex();
        } else {
            pp->m_previousTrack = index(pp->m_currentTrack.row() - 1, 0);
        }
        if (pp->m_currentTrack.row() == rowCount() - 1) {
            pp->m_nextTrack = QPersistentModelIndex();
        } else {
            pp->m_nextTrack = index(pp->m_currentTrack.row() + 1, 0);
        }
        break;
    case Playlist:
        // forward to end or begin when repeating
        if (pp->m_currentTrack.row() == 0) {
            pp->m_previousTrack = index(rowCount() - 1, 0);
        } else {
            pp->m_previousTrack = index(pp->m_currentTrack.row() - 1, 0);
        }
        if (pp->m_currentTrack.row() == rowCount() - 1) {
            pp->m_nextTrack = index(0, 0);
        } else {
            pp->m_nextTrack = index(pp->m_currentTrack.row() + 1, 0);
        }
        break;
    case CurrentTrack:
        pp->m_previousTrack = pp->m_currentTrack;
        pp->m_nextTrack = pp->m_currentTrack;
        break;
    }

    if (pp->m_previousTrack != oldPreviousTrack) {
        Q_EMIT previousTrackChanged(pp->m_previousTrack);
    }

    if (pp->m_nextTrack != oldNextTrack) {
        Q_EMIT nextTrackChanged(pp->m_nextTrack);
    }
}

QVariantList PlaylistProxyModel::getRandomMappingForRestore() const {
    QVariantList randomMapping;

    if (pp->m_shuffleMode != NoShuffle) {
        randomMapping.reserve(pp->m_randomMapping.count());
        for (int i = 0; i < pp->m_randomMapping.count(); ++i) {
            randomMapping.append(QVariant(pp->m_randomMapping[i]));
        }
    }

    return randomMapping;
}

void PlaylistProxyModel::restoreShuffleMode(const Shuffle mode, const QVariantList& mapping) {
    const auto playlistSize = rowCount();

    if (mode == Shuffle::NoShuffle || mapping.count() != playlistSize || !pp->m_randomMapping.isEmpty()) return;

    Q_EMIT layoutAboutToBeChanged({}, QAbstractItemModel::VerticalSortHint);

    QModelIndexList from;
    QModelIndexList to;
    from.reserve(playlistSize);
    to.reserve(playlistSize);

    pp->m_randomMapping.clear();
    pp->m_randomMapping.reserve(playlistSize);

    for (int i = 0; i < playlistSize; ++i) {
        pp->m_randomMapping.append(mapping[i].toInt());
        from.append(index(mapping[i].toInt(), 0));
        to.append(index(i, 0));
    }
    changePersistentIndexList(from, to);

    pp->m_shuffleMode = mode;

    Q_EMIT layoutChanged({}, QAbstractItemModel::VerticalSortHint);
    Q_EMIT shuffleModeChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
}
