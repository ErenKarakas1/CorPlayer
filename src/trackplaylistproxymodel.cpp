#include "trackplaylistproxymodel.h"
#include "playerutils.h"
#include "trackplaylist.h"

#include <QDir>
#include <QFile>
#include <QList>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QItemSelection>
#include <QRandomGenerator>

#include <algorithm>

QList<QUrl> M3UPlaylistParser::fromPlaylist(const QUrl &m3uFile, const QByteArray &content) {
    Q_UNUSED(m3uFile);
    QList<QUrl> result;

    const auto lines = content.split('\n');
    for (const QByteArray &l : lines) {
        const QString &line = QString::fromUtf8(l);
        if (!line.isEmpty() && !line.startsWith(QStringLiteral("#"))) {
            const QUrl &url = line.contains(QStringLiteral("://")) ? QUrl(line) : QUrl::fromLocalFile(line);
            result.append(url);
        }
    }

    return result;
}

QString M3UPlaylistParser::toPlaylist(const QUrl &m3uFile, const QList<QString> &urls) {
    Q_UNUSED(m3uFile);
    QString result;

    for (const QString &line : urls) {
        if (!line.isEmpty()) {
            result += line + QStringLiteral("\n");
        }
    }

    return result;
}

QList<QUrl> PlaylistParser::fromPlaylist(const QUrl &m3uFile, const QByteArray &content) {
    QList<QUrl> result;

    if (m3uFile.isValid() && !m3uFile.isEmpty()) {
        auto mimeType = QMimeDatabase().mimeTypeForUrl(m3uFile);

        if (mimeType.name().contains(QStringLiteral("mpegurl"))) {
            M3UPlaylistParser m3uPlaylistParser;
            result = m3uPlaylistParser.fromPlaylist(m3uFile, content);
        }
    }

    return result;
}

QString PlaylistParser::toPlaylist(const QUrl &m3uFile, const QList<QString> &urls) {
    QString result;

    if (m3uFile.isValid() && !m3uFile.isEmpty()) {
        auto mimeType = QMimeDatabase().mimeTypeForUrl(m3uFile);

        if (mimeType.name().contains(QStringLiteral("mpegurl"))) {
            M3UPlaylistParser m3uPlaylistParser;
            result = m3uPlaylistParser.toPlaylist(m3uFile, urls);
        }
    }

    return result;
}

class TrackPlaylistProxyModelPrivate {
public:
    TrackPlaylist* m_playlistModel;
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

TrackPlaylistProxyModel::TrackPlaylistProxyModel(QObject *parent)
    : QAbstractProxyModel(parent), tpp(std::make_unique<TrackPlaylistProxyModelPrivate>()) {
    tpp->m_randomGenerator.seed(static_cast<unsigned int>(QTime::currentTime().msec()));
}

TrackPlaylistProxyModel::~TrackPlaylistProxyModel() = default;

QModelIndex TrackPlaylistProxyModel::index(int row, int column, const QModelIndex &parent) const {
    if (row < 0 || column < 0 || row > rowCount() - 1) {
        return QModelIndex();
    }
    return createIndex(row, column);

    Q_UNUSED(parent);
}

QModelIndex TrackPlaylistProxyModel::mapFromSource(const QModelIndex &sourceIndex) const {
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    return tpp->m_playlistModel->index(mapRowFromSource(sourceIndex.row()), sourceIndex.column());
}

QItemSelection TrackPlaylistProxyModel::mapSelectionFromSource(const QItemSelection &selection) const {
    QItemSelection mappedSelection;
    for (const QItemSelectionRange &range : selection) {
        QModelIndex proxyTopLeft = mapFromSource(range.topLeft());
        QModelIndex proxyBottomRight = mapFromSource(range.bottomRight());
        mappedSelection.append(QItemSelectionRange(proxyTopLeft, proxyBottomRight));
    }

    return mappedSelection;
}

QItemSelection TrackPlaylistProxyModel::mapSelectionToSource(const QItemSelection &selection) const
{
    QItemSelection sourceSelection;
    for (const QItemSelectionRange &range : selection) {
        QModelIndex sourceTopLeft = mapToSource(range.topLeft());
        QModelIndex sourceBottomRight = mapToSource(range.bottomRight());
        sourceSelection.append(QItemSelectionRange(sourceTopLeft, sourceBottomRight));
    }

    return sourceSelection;
}

QModelIndex TrackPlaylistProxyModel::mapToSource(const QModelIndex &proxyIndex) const {
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }

    return tpp->m_playlistModel->index(mapRowToSource(proxyIndex.row()), proxyIndex.column());
}

int TrackPlaylistProxyModel::mapRowToSource(const int proxyRow) const {
    if (tpp->m_shuffleMode != TrackPlaylistProxyModel::Shuffle::NoShuffle) {
        return tpp->m_randomMapping.at(proxyRow);
    }

    return proxyRow;
}

int TrackPlaylistProxyModel::mapRowFromSource(const int sourceRow) const {
    if (tpp->m_shuffleMode != TrackPlaylistProxyModel::Shuffle::NoShuffle) {
        return tpp->m_randomMapping.indexOf(sourceRow);
    }

    return sourceRow;
}

int TrackPlaylistProxyModel::rowCount(const QModelIndex &parent) const {
    if (tpp->m_shuffleMode != TrackPlaylistProxyModel::Shuffle::NoShuffle) {
        if (parent.isValid()) {
            return 0;
        }
        return tpp->m_randomMapping.count();
    }

    return tpp->m_playlistModel->rowCount(parent);
}

int TrackPlaylistProxyModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QModelIndex TrackPlaylistProxyModel::parent(const QModelIndex &child) const {
    Q_UNUSED(child);
    return QModelIndex();
}

bool TrackPlaylistProxyModel::hasChildren(const QModelIndex &parent) const {
    return (parent.isValid()) ? false : (rowCount() > 0);
}

void TrackPlaylistProxyModel::setPlaylistModel(TrackPlaylist *playlistModel) {
    if (tpp->m_playlistModel) {
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

    if (tpp->m_playlistModel) {
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
}

void TrackPlaylistProxyModel::setSourceModel(QAbstractItemModel *sourceModel) {
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

void TrackPlaylistProxyModel::setRepeatMode(Repeat newMode) {
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

void TrackPlaylistProxyModel::setShuffleMode(const TrackPlaylistProxyModel::Shuffle value) {
    if (tpp->m_shuffleMode != value) {
        auto playlistSize = tpp->m_playlistModel->rowCount();

        if (playlistSize != 0) {
            Q_EMIT layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

            // TODO first, reset playlist to non-random one if it's currently shuffled, but why?
            // if (!tpp->m_randomMapping.isEmpty()) {
            //     Q_ASSERT(tpp->m_shuffleMode != TrackPlaylistProxyModel::Shuffle::NoShuffle);
            //     QModelIndexList from;
            //     from.reserve(playListSize);
            //     QModelIndexList to;
            //     to.reserve(playListSize);
            //     for (int i = 0; i < playListSize; ++i) {
            //         to.append(index(tpp->m_randomMapping.at(i), 0));
            //         from.append(index(i, 0));
            //     }
            //     changePersistentIndexList(from, to);
            //
            //     tpp->m_randomMapping.clear();
            // }
            if (value != TrackPlaylistProxyModel::Shuffle::NoShuffle) {
                QModelIndexList from, to;
                from.reserve(playlistSize);
                to.reserve(playlistSize);

                tpp->m_randomMapping.clear();
                tpp->m_randomMapping.reserve(playlistSize);

                for (int i = 0; i < playlistSize; ++i) {
                    tpp->m_randomMapping.append(i);
                    to.append(index(i, 0));
                }

                if (value != TrackPlaylistProxyModel::Shuffle::Track) {
                    if (playlistSize > 1) {
                        if (currentTrackRow() != 0) {
                            std::swap(tpp->m_randomMapping[0], tpp->m_randomMapping[currentTrackRow()]);
                        }
                        from.append(index(tpp->m_randomMapping.at(0), 0));
                    }
                    // Fisher-Yates algorithm
                    for (int i = 1;  i < playlistSize - 1; ++i) {
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
}

TrackPlaylistProxyModel::Shuffle TrackPlaylistProxyModel::shuffleMode() const {
    return tpp->m_shuffleMode;
}

void TrackPlaylistProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end) {
    if (tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::NoShuffle) {
        beginInsertRows(parent, start, end);
    }
}

void TrackPlaylistProxyModel::sourceRowsInserted(const QModelIndex &parent, int start, int end) {
    if (tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::Track) {
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
                std::transform(tpp->m_randomMapping.begin(), tpp->m_randomMapping.end(), tpp->m_randomMapping.begin(), [=](const int sourceRow) {
                    return sourceRow < start ? sourceRow : sourceRow + newItemsCount;
                });
            }

            const bool enqueueAfterCurrentTrack = mapRowFromSource(start - 1) == tpp->m_currentTrack.row();

            if (enqueueAfterCurrentTrack) {
                // Internally shuffle the new tracks then enqueue them all immediately after the current track
                QList<int> shuffledSourceRows(newItemsCount);
                std::iota(shuffledSourceRows.begin(), shuffledSourceRows.end(), start);
                std::shuffle(shuffledSourceRows.begin(), shuffledSourceRows.end(), tpp->m_randomGenerator);
                const int proxyStart = tpp->m_currentTrack.row() + 1;
                beginInsertRows(parent, proxyStart, proxyStart + newItemsCount - 1);
                for (int sourceRow : shuffledSourceRows) {
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

void TrackPlaylistProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) {
    if (tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::NoShuffle) {
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

void TrackPlaylistProxyModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end) {
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    if (tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::NoShuffle) {
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

void TrackPlaylistProxyModel::sourceRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int destRow) {
    Q_ASSERT(tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::NoShuffle);
    beginMoveRows(parent, start, end, destination, destRow);
}

void TrackPlaylistProxyModel::sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int destRow) {
    Q_ASSERT(tpp->m_shuffleMode == TrackPlaylistProxyModel::Shuffle::NoShuffle);

    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_UNUSED(destination);
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

void TrackPlaylistProxyModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles) {
    qDebug() << "TrackPlaylistProxyModel::sourceDataChanged: topLeft = " << topLeft << ", bottomRight = " << bottomRight;

    auto startSourceRow = topLeft.row();
    auto endSourceRow = bottomRight.row();

    for (int i  = startSourceRow; i <= endSourceRow; ++i) {
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

void TrackPlaylistProxyModel::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last) {
    Q_EMIT headerDataChanged(orientation, first, last);
}

int TrackPlaylistProxyModel::totalTracksDuration() const {
    int time = 0;

    for (int i = 0; i < rowCount(); ++i) {
        time += tpp->m_playlistModel->data(index(i, 0), TrackPlaylist::DurationRole).toTime().msecsSinceStartOfDay();
    }
    return time;
}

int TrackPlaylistProxyModel::remainingTracksDuration() const {
    int time = 0;

    for (int i = tpp->m_currentTrack.row(); i < rowCount(); ++i) {
        time += tpp->m_playlistModel->data(index(i, 0), TrackPlaylist::DurationRole).toTime().msecsSinceStartOfDay();
    }
    return time;
}

int TrackPlaylistProxyModel::remainingTracks() const {
    if (!tpp->m_currentTrack.isValid() || (tpp->m_repeatMode == Repeat::CurrentTrack) || (tpp->m_repeatMode == Repeat::Playlist)) {
        return -1;
    } else {
        return rowCount() - tpp->m_currentTrack.row() - 1;
    }
}

int TrackPlaylistProxyModel::tracksCount() const {
    return rowCount();
}

int TrackPlaylistProxyModel::currentTrackRow() const {
    return tpp->m_currentTrack.row();
}

void TrackPlaylistProxyModel::enqueue(const MetadataFields::MusicMetadataField &newEntry,
    const QString &newEntryTitle, PlayerUtils::PlaylistEnqueueMode enqueueMode,
    PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    qDebug() << "TrackPlaylistProxyModel::enqueue: Enqueueing track with title " << newEntryTitle;
    enqueue({{newEntry, newEntryTitle, {}}}, enqueueMode, triggerPlay);
}

void TrackPlaylistProxyModel::enqueue(const QUrl &entryUrl,
    PlayerUtils::PlaylistEnqueueMode enqueueMode,
    PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    qDebug() << "TrackPlaylistProxyModel::enqueue: Enqueueing track with URL " << entryUrl;
    enqueue({{{{MetadataFields::ElementTypeRole, PlayerUtils::Track}}, {}, entryUrl}}, enqueueMode, triggerPlay);
}

void TrackPlaylistProxyModel::enqueue(const MetadataFields::EntryMetadataList &newEntries,
    PlayerUtils::PlaylistEnqueueMode enqueueMode,
    PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    qDebug() << "TrackPlaylistProxyModel::enqueue: Enqueueing " << newEntries.count() << " tracks";
    if (newEntries.isEmpty()) {
        return;
    }

    tpp->m_triggerPlay = triggerPlay;

    if (enqueueMode == PlayerUtils::ReplacePlaylist) {
        if (rowCount() != 0) {
            clearPlaylist();
        }
    }

    qDebug() << "TrackPlaylistProxyModel::enqueue: Enqueue mode is " << enqueueMode;

    const int enqueueIndex = enqueueMode == PlayerUtils::AfterCurrentTrack ? mapRowToSource(tpp->m_currentTrack.row()) + 1 : -1;
    tpp->m_playlistModel->enqueueMultipleEntries(newEntries, enqueueIndex);
}

void TrackPlaylistProxyModel::trackInError(const QUrl &sourceInError, QMediaPlayer::Error playerError) {
    tpp->m_playlistModel->trackInError(sourceInError, playerError);
}

void TrackPlaylistProxyModel::skipNextTrack(PlayerUtils::SkipReason reason) {
    if (!tpp->m_currentTrack.isValid()) {
        return;
    }

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

void TrackPlaylistProxyModel::skipPreviousTrack(qint64 position) {
    if (!tpp->m_currentTrack.isValid()) {
        return;
    }

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

void TrackPlaylistProxyModel::switchTo(int row) {
    if (!tpp->m_currentTrack.isValid()) {
        return;
    }

    tpp->m_currentTrack = index(row, 0);
    notifyCurrentTrackChanged();
}

void TrackPlaylistProxyModel::removeSelection(QList<int> selection) {
    std::sort(selection.begin(), selection.end());
    std::reverse(selection.begin(), selection.end());

    for (auto item : selection) {
        removeRow(item);
    }
}

void TrackPlaylistProxyModel::removeRow(int row) {
    tpp->m_playlistModel->removeRows(mapRowToSource(row), 1);
}

void TrackPlaylistProxyModel::moveRow(int from, int to) {
    const bool currentTrackIndexChanged = (from < to) ? (from <= tpp->m_currentTrack.row() && tpp->m_currentTrack.row() <= to)
                                                      : (to <= tpp->m_currentTrack.row() && tpp->m_currentTrack.row() <= from);

    if (tpp->m_shuffleMode != TrackPlaylistProxyModel::Shuffle::NoShuffle) {
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

    auto m_oldPreviousTrack = tpp->m_previousTrack;
    auto m_oldNextTrack = tpp->m_nextTrack;

    switch (tpp->m_repeatMode) {
        case Repeat::None:
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
        case Repeat::Playlist:
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
        case Repeat::CurrentTrack:
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

int TrackPlaylistProxyModel::indexForTrackUrl(const QUrl &url) {
    for (int i = 0; i < rowCount(); ++i) {
        const QUrl thisTrackUrl = data(index(i,0), TrackPlaylist::ResourceRole).toUrl();

        if (thisTrackUrl == url) {
            return i;
        }
    }
    return -1;
}

void TrackPlaylistProxyModel::switchToTrackUrl(const QUrl &url, PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    switchTo(indexForTrackUrl(url));

    if (triggerPlay == PlayerUtils::TriggerPlay) {
        Q_EMIT requestPlay();
    }
}

void TrackPlaylistProxyModel::clearPlaylist() {
    if (rowCount() == 0) {
        return;
    }

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
            const auto type = candidateTrack.data(TrackPlaylist::ElementTypeRole).value<PlayerUtils::PlaylistEntryType>();

            if (candidateTrack.isValid() && candidateTrack.data(TrackPlaylist::IsValidRole).toBool() &&
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

bool TrackPlaylistProxyModel::savePlaylist(const QUrl &fileName) {
    QFile outputFile(fileName.toLocalFile());
    auto open = outputFile.open(QFile::WriteOnly);

    if (!open) {
        return false;
    }

    QList<QString> listOfFilePaths;
    
    for (int i = 0; i < rowCount(); ++i) {
        if (data(index(i,0), TrackPlaylist::IsValidRole).toBool()) {
            listOfFilePaths.append(data(index(i,0), TrackPlaylist::ResourceRole).toUrl().toLocalFile());
        }
    }

    PlaylistParser playlistParser;
    QString fileContent = playlistParser.toPlaylist(fileName, listOfFilePaths);

    outputFile.write(fileContent.toUtf8());

    return true;
}

void TrackPlaylistProxyModel::loadPlaylist(const QUrl &fileName) {
    loadPlaylist(fileName, PlayerUtils::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);
}

void TrackPlaylistProxyModel::loadPlaylist(const QUrl &fileName,
                                           PlayerUtils::PlaylistEnqueueMode enqueueMode,
                                           PlayerUtils::PlaylistEnqueueTriggerPlay triggerPlay) {
    resetPartiallyLoaded();
    qDebug() << "Loading playlist from" << fileName;

    QFile inputFile(fileName.toLocalFile());
    bool open = inputFile.open(QFile::ReadOnly | QIODevice::Text);
    qDebug() << "File open:" << open;

    if (!open) {
        Q_EMIT playlistLoadFailed();
    }

    const QByteArray fileContent = inputFile.readAll();

    if (enqueueMode == PlayerUtils::ReplacePlaylist) {
        clearPlaylist();
    }

    tpp->m_loadedPlaylistUrl = fileName;

    MetadataFields::EntryMetadataList newTracks;
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

void TrackPlaylistProxyModel::setPersistentState(const QVariantMap &persistentStateValue)
{
    auto playListIt = persistentStateValue.find(QStringLiteral("playlist"));
    if (playListIt != persistentStateValue.end()) {
        tpp->m_playlistModel->enqueueRestoredEntries(playListIt.value().toList());
    }

    auto shuffleModeStoredValue = persistentStateValue.find(QStringLiteral("shuffleMode"));
    auto shuffleRandomMappingIt = persistentStateValue.find(QStringLiteral("randomMapping"));
    if (shuffleModeStoredValue != persistentStateValue.end() && shuffleRandomMappingIt != persistentStateValue.end()) {
        restoreShuffleMode(shuffleModeStoredValue->value<Shuffle>(), shuffleRandomMappingIt.value().toList());
    }

    auto playerCurrentTrack = persistentStateValue.find(QStringLiteral("currentTrack"));
    if (playerCurrentTrack != persistentStateValue.end()) {
        auto newIndex = index(playerCurrentTrack->toInt(), 0);
        if (newIndex.isValid() && (newIndex != tpp->m_currentTrack)) {
            tpp->m_currentTrack = newIndex;
            notifyCurrentTrackChanged();
        }
    }

    auto repeatPlayStoredValue = persistentStateValue.find(QStringLiteral("repeatPlay"));
    if (repeatPlayStoredValue != persistentStateValue.end() && repeatPlayStoredValue->value<bool>()) {
        setRepeatMode(Repeat::Playlist);
    }

    auto repeatModeStoredValue = persistentStateValue.find(QStringLiteral("repeatMode"));
    if (repeatModeStoredValue != persistentStateValue.end()) {
        setRepeatMode(repeatModeStoredValue->value<Repeat>());
    }

    Q_EMIT persistentStateChanged();
}

QVariantList TrackPlaylistProxyModel::getRandomMappingForRestore() const {
    QVariantList randomMapping;

    if (tpp->m_shuffleMode != TrackPlaylistProxyModel::Shuffle::NoShuffle) {
        randomMapping.reserve(tpp->m_randomMapping.count());
        for (int i = 0; i < tpp->m_randomMapping.count(); ++i) {
            randomMapping.append(QVariant(tpp->m_randomMapping[i]));
        }
    }

    return randomMapping;
}

void TrackPlaylistProxyModel::restoreShuffleMode(TrackPlaylistProxyModel::Shuffle mode, QVariantList mapping) {
    auto playListSize = rowCount();

    if (mode != TrackPlaylistProxyModel::Shuffle::NoShuffle && mapping.count() == playListSize && tpp->m_randomMapping.isEmpty()) {
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

void TrackPlaylistProxyModel::loadLocalFile(MetadataFields::EntryMetadataList &newTracks, QSet<QString> &processedFiles, const QFileInfo &fileInfo) {
    // protection against recursion
    auto canonicalFilePath = fileInfo.canonicalFilePath();
    if (processedFiles.contains(canonicalFilePath)) {
        return;
    }
    processedFiles.insert(canonicalFilePath);

    auto fileUrl = QUrl::fromLocalFile(fileInfo.filePath());
    auto mimeType = tpp->m_mimeDB.mimeTypeForUrl(fileUrl);
    if (fileInfo.isDir()) {
        if (fileInfo.isSymLink()) {
            return;
        }
        loadLocalDirectory(newTracks, processedFiles, fileUrl);
    } else {
        if (!mimeType.name().startsWith(QLatin1String("audio/"))) {
            return;
        }
        if (PlayerUtils::isPlaylist(mimeType)) {
            QFile file(fileInfo.filePath());
            if (!file.open(QIODevice::ReadOnly)) {
                tpp->m_partiallyLoaded = true;
                return;
            }
            loadLocalPlaylist(newTracks, processedFiles, fileUrl, file.readAll());
            return;
        }
        newTracks.push_back({{{MetadataFields::ElementTypeRole, PlayerUtils::FileName}, {MetadataFields::ResourceRole, fileUrl}}, {}, {}});
    }
}

void TrackPlaylistProxyModel::loadLocalPlaylist(MetadataFields::EntryMetadataList &newTracks,
                                                QSet<QString> &processedFiles,
                                                const QUrl &fileName,
                                                const QByteArray &fileContent) {
    PlaylistParser playlistParser;
    QList<QUrl> listOfUrls = playlistParser.fromPlaylist(fileName, fileContent);

    int filtered = filterLocalPlaylist(listOfUrls, fileName);
    if (filtered != 0) {
        tpp->m_partiallyLoaded = true;
    }

    for (const QUrl &oneUrl : std::as_const(listOfUrls)) {
        if (oneUrl.isLocalFile()) {
            QFileInfo fileInfo(oneUrl.toLocalFile());
            loadLocalFile(newTracks, processedFiles, fileInfo);
        } else {
            newTracks.push_back({{{{MetadataFields::ElementTypeRole, PlayerUtils::FileName}, {MetadataFields::ResourceRole, oneUrl}}}, {}, {}});
        }
    }
}

void TrackPlaylistProxyModel::loadLocalDirectory(MetadataFields::EntryMetadataList &newTracks, QSet<QString> &processedFiles, const QUrl &dirName) {
    QDir dirInfo(dirName.toLocalFile());
    const auto fileInfoList = dirInfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Files | QDir::Dirs, QDir::Name);

    for (const auto &fileInfo : fileInfoList) {
        loadLocalFile(newTracks, processedFiles, fileInfo);
    }
}

int TrackPlaylistProxyModel::filterLocalPlaylist(QList<QUrl>& result, const QUrl &playlistUrl) {
    int filtered = 0;

    for (auto iterator = result.begin(); iterator != result.end();) {
        bool exists = true;

        auto &url = *iterator;
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
