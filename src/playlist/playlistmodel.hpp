#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "metadata.hpp"
#include "playerutils.hpp"

#include <QAbstractListModel>
#include <QMediaPlayer>
#include <QQmlEngine>

#include <memory>
#include <utility>

class QDebug;
class PlaylistEntry;

class PlaylistModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum PlayState {
        NotPlaying,
        IsPlaying,
        IsPaused,
    };

    Q_ENUM(PlayState)

    using EntryType = PlayerUtils::PlaylistEntryType;
    using TrackFieldsList = QList<Metadata::TrackFields>;

    explicit PlaylistModel(QObject* parent = nullptr);
    ~PlaylistModel() override;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent,
                  int destinationChild) override;

    void clearPlaylist();
    void enqueueRestoredEntries(const QVariantList& newEntries);
    [[nodiscard]] QVariantList getEntriesForRestore() const;

Q_SIGNALS:
    void addTrackByName(const QString& title, const QString& artist, const QString& album, int trackNumber,
                        int discNumber);

    void addNewEntry(qulonglong newDatabaseId, const QString& entryTitle, PlaylistModel::EntryType databaseIdType);

    void addNewUrl(const QUrl& entryUrl, PlaylistModel::EntryType databaseIdType);

public Q_SLOTS:
    void tracksListAdded(qulonglong newDatabaseId, const QString& entryTitle, PlaylistModel::EntryType databaseIdType,
                         const PlaylistModel::TrackFieldsList& tracks);

    void trackChanged(const Metadata::TrackFields& track);
    void trackRemoved(qulonglong trackId);
    void trackInError(const QUrl& sourceInError, QMediaPlayer::Error playerError);

    /**
     * If `insertAt` is negative or larger than the playlist size, the entries
     * will be enqueued at the end of the playlist.
     */
    void enqueueOneEntry(const Metadata::EntryFields& entryData, int insertAt = -1);
    void enqueueMultipleEntries(const Metadata::EntryFieldsList& entriesData, int insertAt = -1);

private:
    QList<PlaylistEntry> m_entries;
    QList<Metadata::TrackFields> m_trackFields;
};

class PlaylistEntry {
public:
    PlaylistEntry() = default;

    PlaylistEntry(qulonglong id, QString title, QString artist, QString album, QUrl trackUrl, int trackNumber,
                  int discNumber, PlaylistModel::EntryType entryType = PlayerUtils::Unknown);

    explicit PlaylistEntry(const Metadata::TrackFields& track);

    explicit PlaylistEntry(QUrl fileName);

    explicit PlaylistEntry(quint64 id, const QString& entryTitle, PlaylistModel::EntryType type);

    QString m_title;
    QString m_album;
    QString m_artist;
    QUrl m_trackUrl;
    QUrl m_coverImageUrl;
    int m_trackNumber = 0;
    int m_discNumber = 0;
    quint64 m_dbId = 0;
    bool m_isValid = false;
    PlayerUtils::PlaylistEntryType m_entryType = PlayerUtils::Unknown;
    PlaylistModel::PlayState m_isPlaying = PlaylistModel::NotPlaying;
};

#endif // PLAYLISTMODEL_H
