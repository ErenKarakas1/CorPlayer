#ifndef PLAYLISTCOLLECTIONMODEL_HPP
#define PLAYLISTCOLLECTIONMODEL_HPP

#include "metadata.hpp"

#include <QAbstractListModel>

class Library;

class PlaylistCollectionModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Only available through CorPlayer")

public:
    enum Roles {
        PlaylistIdRole = Qt::UserRole + 1,
        NameRole,
        TrackCountRole,
    };
    Q_ENUM(Roles)

    explicit PlaylistCollectionModel(Library* library, QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void createPlaylist(const QString& name);
    Q_INVOKABLE void renamePlaylist(int index, const QString& name);
    Q_INVOKABLE void removePlaylist(int index);
    Q_INVOKABLE [[nodiscard]] QList<quint64> getPlaylistTracks(int index) const;

public Q_SLOTS:
    void onPlaylistModified(quint64 id);

private:
    void loadPlaylists();

    Library* m_library;
    QList<Metadata::PlaylistRecord> m_playlists;
};

#endif // PLAYLISTCOLLECTIONMODEL_HPP
