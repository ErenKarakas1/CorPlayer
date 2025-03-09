#ifndef PLAYLISTMODEL_HPP
#define PLAYLISTMODEL_HPP

#include "metadata.hpp"
#include "playerutils.hpp"

#include <QAbstractListModel>
#include <QMediaPlayer>
#include <QQmlEngine>

#include <memory>

class Library;
class PlaylistEntry;
class PlaylistModelPrivate;

class PlaylistModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Only available through CorPlayer")

public:
    enum Roles {
        IsValidRole = Metadata::Fields::IsValid,
        DatabaseIdRole = Metadata::Fields::DatabaseId,
        TitleRole = Metadata::Fields::Title,
        ArtistRole = Metadata::Fields::Artist,
        AlbumRole = Metadata::Fields::Album,
        ResourceUrlRole = Metadata::Fields::ResourceUrl,
        TrackNumberRole = Metadata::Fields::TrackNumber,
        DiscNumberRole = Metadata::Fields::DiscNumber,
        DurationRole = Metadata::Fields::Duration,
        DurationStringRole = Metadata::Fields::DurationString,
        ElementTypeRole = Metadata::Fields::ElementType,
        IsPlayingRole = Metadata::PlaylistFields::IsPlaying,
    };
    Q_ENUM(Roles)

    enum PlayState {
        NotPlaying,
        IsPlaying,
        IsPaused,
    };
    Q_ENUM(PlayState)

    using EntryType = PlayerUtils::PlaylistEntryType;

    explicit PlaylistModel(Library* library, QObject* parent = nullptr);
    ~PlaylistModel() override;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void clearPlaylist();
    void enqueueRestoredEntries(const QVariantList& newEntries);
    void enqueueMultipleEntries(const Metadata::EntryFieldsList& newEntries, int insertAt = -1);
    void loadTracksFromIds(const QList<quint64>& trackIds);
    [[nodiscard]] QVariantList getEntriesForRestore() const;

Q_SIGNALS:
    void addNewUrl(const QUrl& entryUrl, PlaylistModel::EntryType databaseIdType);

public Q_SLOTS:
    void onTrackModified(quint64 id, const Metadata::TrackFields& track);
    void onTrackRemoved(quint64 id);

private:
    std::unique_ptr<PlaylistModelPrivate> p;
};

#endif // PLAYLISTMODEL_HPP
