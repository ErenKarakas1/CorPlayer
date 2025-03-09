#ifndef TRACKCOLLECTIONMODEL_HPP
#define TRACKCOLLECTIONMODEL_HPP

#include "metadata.hpp"

#include <QAbstractListModel>

class Library;

class TrackCollectionModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Only available through CorPlayer")

public:
    enum Roles {
        IdRole = Metadata::Fields::DatabaseId,
        TitleRole = Metadata::Fields::Title,
        ArtistRole = Metadata::Fields::Artist,
        AlbumRole = Metadata::Fields::Album,
        AlbumArtistRole = Metadata::Fields::AlbumArtist,
        UrlRole = Metadata::Fields::ResourceUrl,
        TrackNumberRole = Metadata::Fields::TrackNumber,
        DiscNumberRole = Metadata::Fields::DiscNumber,
        DurationRole = Metadata::Fields::Duration,
        DurationStringRole = Metadata::Fields::DurationString,
        YearRole = Metadata::Fields::Year,
        GenreRole = Metadata::Fields::Genre,
    };
    Q_ENUM(Roles)

    explicit TrackCollectionModel(Library* library, QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE [[nodiscard]] quint64 getTrackId(int index) const;
    Q_INVOKABLE [[nodiscard]] QUrl getTrackUrl(int index) const;

private Q_SLOTS:
    void refresh();
    void onTrackAdded(quint64 id, const Metadata::TrackFields& track);
    void onTrackModified(quint64 id, const Metadata::TrackFields& track);
    void onTrackRemoved(quint64 id);

private:
    [[nodiscard]] int findTrackIndex(quint64 id) const;

    Library* m_library;
    QList<Metadata::TrackFields> m_tracks;
};

#endif // TRACKCOLLECTIONMODEL_HPP
