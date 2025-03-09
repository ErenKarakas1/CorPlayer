#include "trackplaylistproxymodel.h"

#include "../library/playlistparser.h"
#include "playlist/savedplaylistmodel.hpp"
#include "playlistmodel.hpp"

#include "../playerutils.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QItemSelection>
#include <QList>
#include <QMimeDatabase>
#include <QRandomGenerator>

#include <algorithm>

class TrackPlaylistProxyModelPrivate {
public:
    SavedPlaylistModel* m_savedPlaylistModel;
    PlaylistModel* m_playlistModel;
    QPersistentModelIndex m_previousTrack;
    QPersistentModelIndex m_currentTrack;
    QPersistentModelIndex m_nextTrack;

    bool m_currentTrackWasValid = false;
    QList<int> m_randomMapping;
    QVariantMap m_persistentSettingsForUndo;
    QRandomGenerator m_randomGenerator;
    QMimeDatabase m_mimeDB;
    PlayerUtils::PlaylistEnqueueTriggerPlay m_triggerPlay = PlayerUtils::DoNotTriggerPlay;
    int m_currentPlaylistPosition = -1;

    TrackPlaylistProxyModel::Repeat m_repeatMode = TrackPlaylistProxyModel::Repeat::None;
    TrackPlaylistProxyModel::Shuffle m_shuffleMode = TrackPlaylistProxyModel::Shuffle::NoShuffle;

    bool m_partiallyLoaded = false;
    QUrl m_loadedPlaylistUrl;
};

TrackPlaylistProxyModel::TrackPlaylistProxyModel(QObject* parent)
    : QAbstractProxyModel(parent), tpp(std::make_unique<TrackPlaylistProxyModelPrivate>()) {
    tpp->m_randomGenerator.seed(static_cast<unsigned int>(QTime::currentTime().msec()));
}

TrackPlaylistProxyModel::~TrackPlaylistProxyModel() = default;

QModelIndex TrackPlaylistProxyModel::index(const int row, const int column, const QModelIndex& parent) const {
    if (row < 0 || column < 0 || row > rowCount() - 1) {
        return {};
    }

    return createIndex(row, column);
    Q_UNUSED(parent);
}

QModelIndex TrackPlaylistProxyModel::mapFromSource(const QModelIndex& sourceIndex) const {
    if (!sourceIndex.isValid()) return {};
    return tpp->m_playlistModel->index(mapRowFromSource(sourceIndex.row()), sourceIndex.column());
}

QItemSelection TrackPlaylistProxyModel::mapSelectionFromSource(const QItemSelection& sourceSelection) const {
    QItemSelection proxySelection;
    for (const QItemSelectionRange& range : sourceSelection) {
        const QModelIndex proxyTopLeft = mapFromSource(range.topLeft());
        const QModelIndex proxyBottomRight = mapFromSource(range.bottomRight());
        proxySelection.append(QItemSelectionRange(proxyTopLeft, proxyBottomRight));
    }

    return proxySelection;
}

QItemSelection TrackPlaylistProxyModel::mapSelectionToSource(const QItemSelection& proxySelection) const {
    QItemSelection sourceSelection;
    for (const QItemSelectionRange& range : proxySelection) {
        const QModelIndex sourceTopLeft = mapToSource(range.topLeft());
        const QModelIndex sourceBottomRight = mapToSource(range.bottomRight());
        sourceSelection.append(QItemSelectionRange(sourceTopLeft, sourceBottomRight));
    }

    return sourceSelection;
}

QModelIndex TrackPlaylistProxyModel::mapToSource(const QModelIndex& proxyIndex) const {
    if (!proxyIndex.isValid()) return {};
    return tpp->m_playlistModel->index(mapRowToSource(proxyIndex.row()), proxyIndex.column());
}

int TrackPlaylistProxyModel::mapRowToSource(const int proxyRow) const {
    if (tpp->m_shuffleMode != NoShuffle) {
        return tpp->m_randomMapping.at(proxyRow);
    }
    return proxyRow;
}

int TrackPlaylistProxyModel::mapRowFromSource(const int sourceRow) const {
    if (tpp->m_shuffleMode != NoShuffle) {
        return static_cast<int>(tpp->m_randomMapping.indexOf(sourceRow));
    }
    return sourceRow;
}

int TrackPlaylistProxyModel::rowCount(const QModelIndex& parent) const {
    if (tpp->m_shuffleMode != NoShuffle) {
        if (parent.isValid()) return 0;
        return static_cast<int>(tpp->m_randomMapping.count());
    }
    return tpp->m_playlistModel->rowCount(parent);
}

int TrackPlaylistProxyModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 1;
}

QModelIndex TrackPlaylistProxyModel::parent(const QModelIndex& child) const {
    Q_UNUSED(child);
    return {};
}

bool TrackPlaylistProxyModel::hasChildren(const QModelIndex& parent) const {
    return parent.isValid() ? false : (rowCount() > 0);
}

void TrackPlaylistProxyModel::setPlaylistModel(PlaylistModel* playlistModel) {
    // clang-format off
    if (tpp->m_playlistModel != nullptr) {
        disconnect(playlistModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &TrackPlaylistProxyModel::sourceRowsAboutToBeInserted);
        disconnect(playlistModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &TrackPlaylistProxyModel::sourceRowsAboutToBeRemoved);
        disconnect(playlistModel, &QAbstractItemModel::rowsAboutToBeMoved, this, &TrackPlaylistProxyModel::sourceRowsAboutToBeMoved);
        disconnect(playlistModel, &QAbstractItemModel::rowsInserted, this, &TrackPlaylistProxyModel::sourceRowsInserted);
        disconnect(playlistModel, &QAbstractItemModel::rowsRemoved, this, &TrackPlaylistProxyModel::sourceRowsRemoved);
        disconnect(playlistModel, &QAbstractItemModel::rowsMoved, this, &TrackPlaylistProxyModel::sourceRowsMoved);
        disconnect(playlistModel, &QAbstractItemModel::dataChanged, this, &TrackPlaylistProxyModel::sourceDataChanged);
        disconnect(playlistModel, &QAbstractItemModel::headerDataChanged, this, &TrackPlaylistProxyModel::sourceHeaderDataChanged);
        disconnect(playlistModel, &QAbstractItemModel::layoutAboutToBeChanged, this, &TrackPlaylistProxyModel::sourceLayoutAboutToBeChanged);
        disconnect(playlistModel, &QAbstractItemModel::layoutChanged, this, &TrackPlaylistProxyModel::sourceLayoutChanged);
        disconnect(playlistModel, &QAbstractItemModel::modelAboutToBeReset, this, &TrackPlaylistProxyModel::sourceModelAboutToBeReset);
        disconnect(playlistModel, &QAbstractItemModel::modelReset, this, &TrackPlaylistProxyModel::sourceModelReset);
    }

    tpp->m_playlistModel = playlistModel;
    setSourceModel(playlistModel);

    if (tpp->m_playlistModel != nullptr) {
        connect(playlistModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &TrackPlaylistProxyModel::sourceRowsAboutToBeInserted);
        connect(playlistModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &TrackPlaylistProxyModel::sourceRowsAboutToBeRemoved);
        connect(playlistModel, &QAbstractItemModel::rowsAboutToBeMoved, this, &TrackPlaylistProxyModel::sourceRowsAboutToBeMoved);
        connect(playlistModel, &QAbstractItemModel::rowsInserted, this, &TrackPlaylistProxyModel::sourceRowsInserted);
        connect(playlistModel, &QAbstractItemModel::rowsRemoved, this, &TrackPlaylistProxyModel::sourceRowsRemoved);
        connect(playlistModel, &QAbstractItemModel::rowsMoved, this, &TrackPlaylistProxyModel::sourceRowsMoved);
        connect(playlistModel, &QAbstractItemModel::dataChanged, this, &TrackPlaylistProxyModel::sourceDataChanged);
        connect(playlistModel, &QAbstractItemModel::headerDataChanged, this, &TrackPlaylistProxyModel::sourceHeaderDataChanged);
        connect(playlistModel, &QAbstractItemModel::layoutAboutToBeChanged, this, &TrackPlaylistProxyModel::sourceLayoutAboutToBeChanged);
        connect(playlistModel, &QAbstractItemModel::layoutChanged, this, &TrackPlaylistProxyModel::sourceLayoutChanged);
        connect(playlistModel, &QAbstractItemModel::modelAboutToBeReset, this, &TrackPlaylistProxyModel::sourceModelAboutToBeReset);
        connect(playlistModel, &QAbstractItemModel::modelReset, this, &TrackPlaylistProxyModel::sourceModelReset);
    }
    // clang-format on
}

void TrackPlaylistProxyModel::setSourceModel(QAbstractItemModel* sourceModel) {
    QAbstractProxyModel::setSourceModel(sourceModel);
}

QPersistentModelIndex TrackPlaylistProxyModel::previousTrack() const {
    return tpp->m_previousTrack;
}

QPersistentModelIndex TrackPlaylistProxyModel::currentTrack() const {
    return tpp->m_currentTrack;
}

QPersistentModelIndex TrackPlaylistProxyModel::nextTrack() const {
    return tpp->m_nextTrack;
}

void TrackPlaylistProxyModel::setRepeatMode(const Repeat newMode) {
    if (tpp->m_repeatMode != newMode) {
        tpp->m_repeatMode = newMode;

        Q_EMIT repeatModeChanged();
        Q_EMIT remainingTracksChanged();
        Q_EMIT remainingTracksDurationChanged();
        Q_EMIT persistentStateChanged();

        determineAndNotifyPreviousAndNextTracks();
    }
}

TrackPlaylistProxyModel::Repeat TrackPlaylistProxyModel::repeatMode() const {
    return tpp->m_repeatMode;
}

void TrackPlaylistProxyModel::setShuffleMode(const Shuffle value) {
    if (tpp->m_shuffleMode == value) return;

    auto playlistSize = tpp->m_playlistModel->rowCount();

    if (playlistSize != 0) {
        Q_EMIT layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

        if (!tpp->m_randomMapping.isEmpty()) {
            Q_ASSERT(tpp->m_shuffleMode != TrackPlaylistProxyModel::Shuffle::NoShuffle);
            QModelIndexList from;
            from.reserve(playlistSize);
            QModelIndexList to;
            to.reserve(playlistSize);
            for (int i = 0; i < playlistSize; ++i) {
                to.append(index(tpp->m_randomMapping.at(i), 0));
                from.append(index(i, 0));
            }
            changePersistentIndexList(from, to);

            tpp->m_randomMapping.clear();
        }
        if (value != Shuffle::NoShuffle) {
            QModelIndexList from, to;
            from.reserve(playlistSize);
            to.reserve(playlistSize);

            tpp->m_randomMapping.clear();
            tpp->m_randomMapping.reserve(playlistSize);

            for (int i = 0; i < playlistSize; ++i) {
                tpp->m_randomMapping.append(i);
                to.append(index(i, 0));
            }

            if (value != Shuffle::Track) {
                if (playlistSize > 1) {
                    if (currentTrackRow() != 0) {
                        std::swap(tpp->m_randomMapping[0], tpp->m_randomMapping[currentTrackRow()]);
                    }
                    from.append(index(tpp->m_randomMapping.at(0), 0));
                }
                // Fisher-Yates algorithm
                for (int i = 1; i < playlistSize - 1; ++i) {
                    const int swapIndex = tpp->m_randomGenerator.bounded(i, playlistSize);
                    std::swap(tpp->m_randomMapping[i], tpp->m_randomMapping[swapIndex]);
                    from.append(index(tpp->m_randomMapping.at(i), 0));
                }
                from.append(index(tpp->m_randomMapping.at(playlistSize - 1), 0));
            }
            changePersistentIndexList(from, to);
        }

        tpp->m_currentPlaylistPosition = tpp->m_currentTrack.row();
        tpp->m_shuffleMode = value;

        Q_EMIT layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

        determineAndNotifyPreviousAndNextTracks();
    } else {
        tpp->m_shuffleMode = value;
    }

    Q_EMIT shuffleModeChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
    Q_EMIT persistentStateChanged();
}

TrackPlaylistProxyModel::Shuffle TrackPlaylistProxyModel::shuffleMode() const {
    return tpp->m_shuffleMode;
}

void TrackPlaylistProxyModel::sourceRowsAboutToBeInserted(const QModelIndex& parent, const int start, const int end) {
    if (tpp->m_shuffleMode == NoShuffle) {
        beginInsertRows(parent, start, end);
    }
}

void TrackPlaylistProxyModel::sourceRowsInserted(const QModelIndex& parent, const int start, const int end) {
    if (tpp->m_shuffleMode == Track) {
        const auto newItemsCount = end - start + 1;
        tpp->m_randomMapping.reserve(rowCount() + newItemsCount);

        if (rowCount() == 0) {
            beginInsertRows(parent, start, end);

            for (int i = 0; i < newItemsCount; ++i) {
                const auto random = tpp->m_randomGenerator.bounded(tpp->m_randomMapping.count() + 1);
                tpp->m_randomMapping.insert(random, start + i);
            }
            endInsertRows();
        } else {
            if (start <= rowCount()) {
                std::ranges::transform(tpp->m_randomMapping, tpp->m_randomMapping.begin(), [=](const int sourceRow) {
                    return sourceRow < start ? sourceRow : sourceRow + newItemsCount;
                });
            }

            const bool enqueueAfterCurrentTrack = mapRowFromSource(start - 1) == tpp->m_currentTrack.row();

            if (enqueueAfterCurrentTrack) {
                // Internally shuffle the new tracks then enqueue them all immediately after the current track
                QList<int> shuffledSourceRows(newItemsCount);
                std::iota(shuffledSourceRows.begin(), shuffledSourceRows.end(), start);
                std::ranges::shuffle(shuffledSourceRows, tpp->m_randomGenerator);

                const int proxyStart = tpp->m_currentTrack.row() + 1;
                beginInsertRows(parent, proxyStart, proxyStart + newItemsCount - 1);
                for (const int sourceRow : shuffledSourceRows) {
                    tpp->m_randomMapping.insert(proxyStart, sourceRow);
                }
                endInsertRows();
            } else {
                const int lowerBound = tpp->m_currentTrack.row() + 1;
                for (int i = 0; i < newItemsCount; ++i) {
                    const auto random = tpp->m_randomGenerator.bounded(lowerBound, rowCount() + 1);
                    beginInsertRows(parent, random, random);
                    tpp->m_randomMapping.insert(random, start + i);
                    endInsertRows();
                }
            }
        }
    } else {
        endInsertRows();
    }

    if (tpp->m_currentTrack.isValid()) {
        tpp->m_currentPlaylistPosition = tpp->m_currentTrack.row();
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

void TrackPlaylistProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex& parent, const int start, const int end) {
    if (tpp->m_shuffleMode == NoShuffle) {
        if (end - start + 1 == rowCount()) {
            beginRemoveRows(parent, start, end);
            tpp->m_randomMapping.clear();
            endRemoveRows();
        }
        int row = 0;
        auto it = tpp->m_randomMapping.begin();
        while (it != tpp->m_randomMapping.end()) {
            if (*it >= start && *it <= end) {
                beginRemoveRows(parent, row, row);
                it = tpp->m_randomMapping.erase(it);
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
        tpp->m_currentTrackWasValid = tpp->m_currentTrack.isValid();
        beginRemoveRows(parent, start, end);
    }
}

void TrackPlaylistProxyModel::sourceRowsRemoved(const QModelIndex& parent, const int start, const int end) {
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    if (tpp->m_shuffleMode == NoShuffle) {
        endRemoveRows();
    }

    if (tpp->m_currentTrack.isValid()) {
        tpp->m_currentPlaylistPosition = tpp->m_currentTrack.row();
    } else {
        tpp->m_currentTrack = index(tpp->m_currentPlaylistPosition, 0);

        if (tpp->m_currentTrack.isValid() && tpp->m_currentTrackWasValid) {
            notifyCurrentTrackChanged();
        } else {
            if (!tpp->m_currentTrack.isValid()) {
                Q_EMIT playlistFinished();
                determineTracks();
                if (!tpp->m_currentTrack.isValid() && tpp->m_currentTrackWasValid) {
                    notifyCurrentTrackChanged();
                }
            }
        }
    }

    if (!tpp->m_nextTrack.isValid() || !tpp->m_previousTrack.isValid()) {
        determineAndNotifyPreviousAndNextTracks();
    }

    Q_EMIT tracksCountChanged();
    Q_EMIT totalTracksDurationChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
    Q_EMIT persistentStateChanged();
}

void TrackPlaylistProxyModel::sourceRowsAboutToBeMoved(const QModelIndex& parent, const int start, const int end,
                                                       const QModelIndex& destParent, const int destRow) {
    Q_ASSERT(tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::NoShuffle);
    beginMoveRows(parent, start, end, destParent, destRow);
}

void TrackPlaylistProxyModel::sourceRowsMoved(const QModelIndex& parent, const int start, const int end,
                                              const QModelIndex& destParent, const int destRow) {
    Q_ASSERT(tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::NoShuffle);

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

void TrackPlaylistProxyModel::sourceModelAboutToBeReset() {
    beginResetModel();
}

void TrackPlaylistProxyModel::sourceModelReset() {
    endResetModel();
}

void TrackPlaylistProxyModel::sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                                const QList<int>& roles) {
    const auto startSourceRow = topLeft.row();
    const auto endSourceRow = bottomRight.row();

    for (int i = startSourceRow; i <= endSourceRow; ++i) {
        Q_EMIT dataChanged(index(mapRowFromSource(i), 0), index(mapRowFromSource(i), 0), roles);
        Q_EMIT remainingTracksDurationChanged();
        Q_EMIT totalTracksDurationChanged();

        if (i == tpp->m_currentTrack.row()) {
            Q_EMIT currentTrackDataChanged();
        } else if (i == tpp->m_previousTrack.row()) {
            Q_EMIT previousTrackDataChanged();
        } else if (i == tpp->m_nextTrack.row()) {
            Q_EMIT nextTrackDataChanged();
        }
        determineTracks();
    }
}

void TrackPlaylistProxyModel::sourceLayoutAboutToBeChanged() {
    Q_EMIT layoutAboutToBeChanged();
}

void TrackPlaylistProxyModel::sourceLayoutChanged() {
    Q_EMIT layoutChanged();
}

void TrackPlaylistProxyModel::sourceHeaderDataChanged(const Qt::Orientation orientation, const int first,
                                                      const int last) {
    Q_EMIT headerDataChanged(orientation, first, last);
}

int TrackPlaylistProxyModel::totalTracksDuration() const {
    int time = 0;

    for (int i = 0; i < rowCount(); ++i) {
        time += tpp->m_playlistModel->data(index(i, 0), Metadata::Fields::Duration).toTime().msecsSinceStartOfDay();
    }
    return time;
}

int TrackPlaylistProxyModel::remainingTracksDuration() const {
    int time = 0;

    for (int i = tpp->m_currentTrack.row(); i < rowCount(); ++i) {
        time += tpp->m_playlistModel->data(index(i, 0), Metadata::Fields::Duration).toTime().msecsSinceStartOfDay();
    }
    return time;
}

int TrackPlaylistProxyModel::remainingTracks() const {
    if (!tpp->m_currentTrack.isValid() || (tpp->m_repeatMode == CurrentTrack) || (tpp->m_repeatMode == Playlist)) {
        return -1;
    }

    return rowCount() - tpp->m_currentTrack.row() - 1;
}

int TrackPlaylistProxyModel::tracksCount() const {
    return rowCount();
}

int TrackPlaylistProxyModel::currentTrackRow() const {
    return tpp->m_currentTrack.row();
}

void TrackPlaylistProxyModel::enqueue(const Metadata::TrackFields& newEntry, const QString& newEntryTitle,
                                      const PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                      const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    enqueue({{.trackFields = newEntry, .title = newEntryTitle, .url = {}}}, enqueueMode, triggerPlay);
}

void TrackPlaylistProxyModel::enqueue(const QUrl& entryUrl, const PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                      const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    auto entry = Metadata::TrackFields();
    entry.insert(Metadata::Fields::ElementType, PlayerUtils::Track);
    enqueue({{.trackFields = entry, .title = {}, .url = entryUrl}}, enqueueMode, triggerPlay);
}

void TrackPlaylistProxyModel::enqueue(const Metadata::EntryFieldsList& newEntries,
                                      const PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                      const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    if (newEntries.isEmpty()) return;

    tpp->m_triggerPlay = triggerPlay;

    if (enqueueMode == PlayerUtils::ReplacePlaylist) {
        if (rowCount() != 0) {
            clearPlaylist();
        }
    }

    const int enqueueIndex =
        enqueueMode == PlayerUtils::AfterCurrentTrack ? mapRowToSource(tpp->m_currentTrack.row()) + 1 : -1;
    tpp->m_playlistModel->enqueueMultipleEntries(newEntries, enqueueIndex);
}

void TrackPlaylistProxyModel::trackInError(const QUrl& sourceInError, const QMediaPlayer::Error playerError) const {
    tpp->m_playlistModel->trackInError(sourceInError, playerError);
}

void TrackPlaylistProxyModel::skipNextTrack(const PlayerUtils::SkipReason reason) {
    if (!tpp->m_currentTrack.isValid()) return;

    if (tpp->m_repeatMode == Repeat::CurrentTrack && reason == PlayerUtils::SkipReason::Automatic) {
        tpp->m_currentTrack = index(tpp->m_currentTrack.row(), 0);
    } else if (tpp->m_currentTrack.row() >= rowCount() - 1) {
        switch (tpp->m_repeatMode) {
        case Repeat::CurrentTrack:
        case Repeat::Playlist:
            tpp->m_currentTrack = index(0, 0);
            break;
        case Repeat::None:
            tpp->m_currentTrack = index(0, 0);
            Q_EMIT playlistFinished();
            break;
        }
    } else {
        tpp->m_currentTrack = index(tpp->m_currentTrack.row() + 1, 0);
    }

    notifyCurrentTrackChanged();
}

void TrackPlaylistProxyModel::skipPreviousTrack(const qint64 position) {
    if (!tpp->m_currentTrack.isValid()) return;

    if (position > m_seekToBeginningDelay) {
        Q_EMIT seek(0);
        return;
    }

    if (tpp->m_currentTrack.row() == 0) {
        if (tpp->m_repeatMode == Repeat::CurrentTrack || tpp->m_repeatMode == Repeat::Playlist) {
            tpp->m_currentTrack = index(rowCount() - 1, 0);
        } else {
            return;
        }
    } else {
        tpp->m_currentTrack = index(tpp->m_currentTrack.row() - 1, 0);
    }

    notifyCurrentTrackChanged();
}

void TrackPlaylistProxyModel::switchTo(const int row) {
    if (!tpp->m_currentTrack.isValid()) return;

    tpp->m_currentTrack = index(row, 0);
    notifyCurrentTrackChanged();
}

// TODO: can't pass as reference?
void TrackPlaylistProxyModel::removeSelection(QList<int> selection) const {
    std::ranges::sort(selection);
    std::ranges::reverse(selection);

    for (const auto item : selection) {
        removeRow(item);
    }
}

void TrackPlaylistProxyModel::removeRow(const int row) const {
    tpp->m_playlistModel->removeRows(mapRowToSource(row), 1);
}

void TrackPlaylistProxyModel::moveRow(const int from, const int to) {
    const bool currentTrackIndexChanged = (from < to)
                                            ? (from <= tpp->m_currentTrack.row() && tpp->m_currentTrack.row() <= to)
                                            : (to <= tpp->m_currentTrack.row() && tpp->m_currentTrack.row() <= from);

    if (tpp->m_shuffleMode != NoShuffle) {
        beginMoveRows({}, from, from, {}, from < to ? to + 1 : to);
        tpp->m_randomMapping.move(from, to);
        endMoveRows();
    } else {
        tpp->m_playlistModel->moveRows({}, from, 1, {}, from < to ? to + 1 : to);
    }

    if (currentTrackIndexChanged) {
        notifyCurrentTrackRowChanged();
    }
}

void TrackPlaylistProxyModel::notifyCurrentTrackRowChanged() {
    if (tpp->m_currentTrack.isValid()) {
        tpp->m_currentPlaylistPosition = tpp->m_currentTrack.row();
    } else {
        tpp->m_currentPlaylistPosition = -1;
    }

    determineAndNotifyPreviousAndNextTracks();

    Q_EMIT currentTrackRowChanged();
    Q_EMIT remainingTracksChanged();
    Q_EMIT remainingTracksDurationChanged();
}

void TrackPlaylistProxyModel::notifyCurrentTrackChanged() {
    notifyCurrentTrackRowChanged();
    Q_EMIT currentTrackChanged(tpp->m_currentTrack);
}

void TrackPlaylistProxyModel::determineAndNotifyPreviousAndNextTracks() {
    if (!tpp->m_currentTrack.isValid()) {
        tpp->m_previousTrack = QPersistentModelIndex();
        tpp->m_nextTrack = QPersistentModelIndex();
    }

    const auto m_oldPreviousTrack = tpp->m_previousTrack;
    const auto m_oldNextTrack = tpp->m_nextTrack;

    switch (tpp->m_repeatMode) {
    case None:
        // return nothing if no tracks available
        if (tpp->m_currentTrack.row() == 0) {
            tpp->m_previousTrack = QPersistentModelIndex();
        } else {
            tpp->m_previousTrack = index(tpp->m_currentTrack.row() - 1, 0);
        }

        if (tpp->m_currentTrack.row() == rowCount() - 1) {
            tpp->m_nextTrack = QPersistentModelIndex();
        } else {
            tpp->m_nextTrack = index(tpp->m_currentTrack.row() + 1, 0);
        }

        break;
    case Playlist:
        // forward to end or begin when repeating
        if (tpp->m_currentTrack.row() == 0) {
            tpp->m_previousTrack = index(rowCount() - 1, 0);
        } else {
            tpp->m_previousTrack = index(tpp->m_currentTrack.row() - 1, 0);
        }

        if (tpp->m_currentTrack.row() == rowCount() - 1) {
            tpp->m_nextTrack = index(0, 0);
        } else {
            tpp->m_nextTrack = index(tpp->m_currentTrack.row() + 1, 0);
        }

        break;
    case CurrentTrack:
        tpp->m_previousTrack = index(tpp->m_currentTrack.row(), 0);
        tpp->m_nextTrack = index(tpp->m_currentTrack.row(), 0);
        break;
    }

    if (tpp->m_previousTrack != m_oldPreviousTrack) {
        Q_EMIT previousTrackChanged(tpp->m_previousTrack);
    }

    if (tpp->m_nextTrack != m_oldNextTrack) {
        Q_EMIT nextTrackChanged(tpp->m_nextTrack);
    }
}

int TrackPlaylistProxyModel::indexForTrackUrl(const QUrl& url) const {
    for (int i = 0; i < rowCount(); ++i) {
        const QUrl thisTrackUrl = data(index(i, 0), Metadata::Fields::ResourceUrl).toUrl();

        if (thisTrackUrl == url) {
            return i;
        }
    }
    return -1;
}

void TrackPlaylistProxyModel::switchToTrackUrl(const QUrl& url,
                                               const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    switchTo(indexForTrackUrl(url));

    if (triggerPlay == PlayerUtils::TriggerPlay) {
        Q_EMIT requestPlay();
    }
}

void TrackPlaylistProxyModel::clearPlaylist() {
    if (rowCount() == 0) return;

    tpp->m_persistentSettingsForUndo = persistentState();
    tpp->m_currentPlaylistPosition = -1;
    tpp->m_currentTrack = QPersistentModelIndex{};

    notifyCurrentTrackChanged();
    tpp->m_playlistModel->clearPlaylist();

    Q_EMIT clearPlaylistPlayer();
    Q_EMIT displayUndoNotification();
}

void TrackPlaylistProxyModel::undoClearPlaylist() {
    tpp->m_playlistModel->clearPlaylist();

    setPersistentState(tpp->m_persistentSettingsForUndo);
    Q_EMIT undoClearPlaylistPlayer();
}

void TrackPlaylistProxyModel::determineTracks() {
    if (!tpp->m_currentTrack.isValid() || tpp->m_currentPlaylistPosition != tpp->m_currentTrack.row()) {
        for (int row = 0; row < rowCount(); ++row) {
            auto candidateTrack = index(row, 0);
            const auto type =
                candidateTrack.data(Metadata::Fields::ElementType).value<PlayerUtils::PlaylistEntryType>();

            if (candidateTrack.isValid() && candidateTrack.data(Metadata::Fields::IsValid).toBool() &&
                (type == PlayerUtils::Track || type == PlayerUtils::FileName)) {
                tpp->m_currentTrack = candidateTrack;
                notifyCurrentTrackChanged();

                if (tpp->m_triggerPlay == PlayerUtils::TriggerPlay) {
                    tpp->m_triggerPlay = PlayerUtils::DoNotTriggerPlay;
                    Q_EMIT ensurePlay();
                }
                break;
            }
        }
    }
    if (!tpp->m_nextTrack.isValid() || !tpp->m_previousTrack.isValid()) {
        determineAndNotifyPreviousAndNextTracks();
    }
}

bool TrackPlaylistProxyModel::savePlaylist(const QUrl& fileName) const {
    QFile outputFile(fileName.toLocalFile());
    auto open = outputFile.open(QFile::WriteOnly);

    if (!open) {
        return false;
    }

    QList<QString> listOfFilePaths;

    for (int i = 0; i < rowCount(); ++i) {
        if (data(index(i, 0), Metadata::Fields::IsValid).toBool()) {
            listOfFilePaths.append(data(index(i, 0), Metadata::Fields::ResourceUrl).toUrl().toLocalFile());
        }
    }

    const QString fileContent = PlaylistParser::toPlaylist(fileName, listOfFilePaths);
    outputFile.write(fileContent.toUtf8());

    return true;
}

void TrackPlaylistProxyModel::loadPlaylistFromDatabase(const quint64 id) {
    clearPlaylist();

    const auto& trackIds = tpp->m_savedPlaylistModel->getTrackIds(id);

    Metadata::EntryFieldsList newTracks;
    newTracks.reserve(trackIds.size());

    for (const auto& trackId : trackIds) {
        auto entry = Metadata::TrackFields();
        entry.insert(Metadata::Fields::ElementType, PlayerUtils::Track);
        entry.insert(Metadata::Fields::DatabaseId, trackId);
        newTracks.emplace_back(Metadata::EntryFields{.trackFields = entry, .title = {}, .url = {}});
    }

    enqueue(newTracks, PlayerUtils::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);
}

void TrackPlaylistProxyModel::loadPlaylist(const QUrl& fileName) {
    loadPlaylist(fileName, PlayerUtils::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);
}

void TrackPlaylistProxyModel::loadPlaylist(const QUrl& fileName, const PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                           const PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    resetPartiallyLoaded();

    QFile inputFile(fileName.toLocalFile());
    const bool open = inputFile.open(QFile::ReadOnly | QIODevice::Text);

    if (!open) {
        Q_EMIT playlistLoadFailed();
    }

    const QByteArray fileContent = inputFile.readAll();

    if (enqueueMode == PlayerUtils::ReplacePlaylist) {
        clearPlaylist();
    }

    tpp->m_loadedPlaylistUrl = fileName;

    Metadata::EntryFieldsList newTracks;
    QSet<QString> processedFiles{QFileInfo(inputFile).canonicalFilePath()};
    loadLocalPlaylist(newTracks, processedFiles, fileName, fileContent);

    enqueue(newTracks, enqueueMode, triggerPlay);

    Q_EMIT persistentStateChanged();
    Q_EMIT playlistLoaded();
    Q_EMIT partiallyLoadedChanged();
    Q_EMIT canOpenLoadedPlaylistChanged();
}

QVariantMap TrackPlaylistProxyModel::persistentState() const {
    QVariantMap currentState;

    currentState[QStringLiteral("playlist")] = tpp->m_playlistModel->getEntriesForRestore();
    currentState[QStringLiteral("shuffleMode")] = tpp->m_shuffleMode;
    currentState[QStringLiteral("randomMapping")] = getRandomMappingForRestore();
    currentState[QStringLiteral("currentTrack")] = tpp->m_currentPlaylistPosition;
    currentState[QStringLiteral("repeatMode")] = tpp->m_repeatMode;

    return currentState;
}

void TrackPlaylistProxyModel::setPersistentState(const QVariantMap& persistentState) {
    auto playListIt = persistentState.find(QStringLiteral("playlist"));
    if (playListIt != persistentState.end()) {
        tpp->m_playlistModel->enqueueRestoredEntries(playListIt.value().toList());
    }

    const auto shuffleModeStoredValue = persistentState.find(QStringLiteral("shuffleMode"));
    auto shuffleRandomMappingIt = persistentState.find(QStringLiteral("randomMapping"));
    if (shuffleModeStoredValue != persistentState.end() && shuffleRandomMappingIt != persistentState.end()) {
        restoreShuffleMode(shuffleModeStoredValue->value<Shuffle>(), shuffleRandomMappingIt.value().toList());
    }

    auto playerCurrentTrack = persistentState.find(QStringLiteral("currentTrack"));
    if (playerCurrentTrack != persistentState.end()) {
        auto newIndex = index(playerCurrentTrack->toInt(), 0);
        if (newIndex.isValid() && (newIndex != tpp->m_currentTrack)) {
            tpp->m_currentTrack = newIndex;
            notifyCurrentTrackChanged();
        }
    }

    auto repeatPlayStoredValue = persistentState.find(QStringLiteral("repeatPlay"));
    if (repeatPlayStoredValue != persistentState.end() && repeatPlayStoredValue->value<bool>()) {
        setRepeatMode(Repeat::Playlist);
    }

    auto repeatModeStoredValue = persistentState.find(QStringLiteral("repeatMode"));
    if (repeatModeStoredValue != persistentState.end()) {
        setRepeatMode(repeatModeStoredValue->value<Repeat>());
    }

    Q_EMIT persistentStateChanged();
}

QVariantList TrackPlaylistProxyModel::getRandomMappingForRestore() const {
    QVariantList randomMapping;

    if (tpp->m_shuffleMode != NoShuffle) {
        randomMapping.reserve(tpp->m_randomMapping.count());
        for (int i = 0; i < tpp->m_randomMapping.count(); ++i) {
            randomMapping.append(QVariant(tpp->m_randomMapping[i]));
        }
    }

    return randomMapping;
}

void TrackPlaylistProxyModel::restoreShuffleMode(const Shuffle mode, QVariantList mapping) {
    auto playListSize = rowCount();

    if (mode != NoShuffle && mapping.count() == playListSize && tpp->m_randomMapping.isEmpty()) {
        Q_EMIT layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

        QModelIndexList from, to;
        from.reserve(playListSize);
        to.reserve(playListSize);

        tpp->m_randomMapping.clear();
        tpp->m_randomMapping.reserve(playListSize);

        for (int i = 0; i < playListSize; ++i) {
            tpp->m_randomMapping.append(mapping[i].toInt());
            from.append(index(mapping[i].toInt(), 0));
            to.append(index(i, 0));
        }
        changePersistentIndexList(from, to);

        tpp->m_shuffleMode = mode;

        Q_EMIT layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
        Q_EMIT shuffleModeChanged();
        Q_EMIT remainingTracksChanged();
        Q_EMIT remainingTracksDurationChanged();
    }
}

bool TrackPlaylistProxyModel::partiallyLoaded() const {
    return tpp->m_partiallyLoaded;
}

bool TrackPlaylistProxyModel::canOpenLoadedPlaylist() const {
    return tpp->m_loadedPlaylistUrl.isValid() && tpp->m_loadedPlaylistUrl.isLocalFile();
}

void TrackPlaylistProxyModel::resetPartiallyLoaded() {
    tpp->m_partiallyLoaded = false;

    Q_EMIT partiallyLoadedChanged();
}

void TrackPlaylistProxyModel::loadLocalFile(Metadata::EntryFieldsList& newTracks, QSet<QString>& processedFiles,
                                            const QFileInfo& fileInfo) {
    // protection against recursion
    const auto canonicalFilePath = fileInfo.canonicalFilePath();
    if (processedFiles.contains(canonicalFilePath)) return;

    processedFiles.insert(canonicalFilePath);

    const auto fileUrl = QUrl::fromLocalFile(fileInfo.filePath());
    const auto mimeType = tpp->m_mimeDB.mimeTypeForUrl(fileUrl);
    if (fileInfo.isDir()) {
        if (fileInfo.isSymLink()) return;
        loadLocalDirectory(newTracks, processedFiles, fileUrl);
    } else {
        if (!mimeType.name().startsWith(QLatin1String("audio/"))) return;

        if (PlayerUtils::isPlaylist(mimeType)) {
            QFile file(fileInfo.filePath());
            if (!file.open(QIODevice::ReadOnly)) {
                tpp->m_partiallyLoaded = true;
                return;
            }
            loadLocalPlaylist(newTracks, processedFiles, fileUrl, file.readAll());
            return;
        }
        auto entry = Metadata::TrackFields();
        entry.insert(Metadata::Fields::ElementType, PlayerUtils::FileName);
        entry.insert(Metadata::Fields::ResourceUrl, fileUrl);
        newTracks.emplace_back(Metadata::EntryFields{.trackFields = entry, .title = {}, .url = {}});
    }
}

void TrackPlaylistProxyModel::loadLocalPlaylist(Metadata::EntryFieldsList& newTracks, QSet<QString>& processedFiles,
                                                const QUrl& fileName, const QByteArray& fileContent) {
    QList<QUrl> listOfUrls = PlaylistParser::fromPlaylist(fileName, fileContent);

    const int filtered = filterLocalPlaylist(listOfUrls, fileName);
    if (filtered != 0) {
        tpp->m_partiallyLoaded = true;
    }

    for (const QUrl& oneUrl : std::as_const(listOfUrls)) {
        if (oneUrl.isLocalFile()) {
            const QFileInfo fileInfo(oneUrl.toLocalFile());
            loadLocalFile(newTracks, processedFiles, fileInfo);
        } else {
            auto entry = Metadata::TrackFields();
            entry.insert(Metadata::Fields::ElementType, PlayerUtils::FileName);
            entry.insert(Metadata::Fields::ResourceUrl, oneUrl);
            newTracks.emplace_back(Metadata::EntryFields{entry, {}, {}});
        }
    }
}

void TrackPlaylistProxyModel::loadLocalDirectory(Metadata::EntryFieldsList& newTracks, QSet<QString>& processedFiles,
                                                 const QUrl& dirName) {
    const QDir dirInfo(dirName.toLocalFile());
    const auto fileInfoList =
        dirInfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Files | QDir::Dirs, QDir::Name);

    for (const auto& fileInfo : fileInfoList) {
        loadLocalFile(newTracks, processedFiles, fileInfo);
    }
}

int TrackPlaylistProxyModel::filterLocalPlaylist(QList<QUrl>& result, const QUrl& playlistUrl) {
    int filtered = 0;

    for (auto iterator = result.begin(); iterator != result.end();) {
        bool exists = true;

        auto& url = *iterator;
        if (url.isLocalFile()) {
            QString file = url.toLocalFile();

            QFileInfo fileInfo(file);
            if (playlistUrl.isLocalFile() && fileInfo.isRelative()) {
                auto absoluteDir = QFileInfo(playlistUrl.toLocalFile()).absoluteDir();
                if (fileInfo.isDir()) {
                    file = absoluteDir.absolutePath() + QDir::separator() + fileInfo.path();
                } else {
                    file = absoluteDir.absoluteFilePath(file);
                }
                fileInfo.setFile(file);
                url = QUrl::fromLocalFile(file);
            }

            exists = fileInfo.exists();
        }

        if (exists) {
            ++iterator;
        } else {
            ++filtered;
            iterator = result.erase(iterator);
        }
    }

    return filtered;
}
