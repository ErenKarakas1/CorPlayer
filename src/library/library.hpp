#ifndef LIBRARY_HPP
#define LIBRARY_HPP

#include "database/playlistdatabase.h"
#include "database/trackdatabase.h"
#include "metadata.hpp"

#include <QMutex>
#include <QObject>
#include <QUrl>

class FileScanner;

class Library : public QObject {
    Q_OBJECT

public:
    explicit Library(QObject* parent = nullptr);
    ~Library() override;

    void initialize(const std::shared_ptr<DbConnectionPool>& pool);

    [[nodiscard]] QList<quint64> addTracksFromUrls(const QList<QUrl>& urls);
    [[nodiscard]] quint64 addTrackFromUrl(const QUrl& url);
    [[nodiscard]] Metadata::TrackFields getTrackById(quint64 id) const;
    void updateTrack(const Metadata::TrackFields& track);
    void removeTrack(quint64 id);

    [[nodiscard]] quint64 createPlaylistFromUrls(const QString& name, const QList<QUrl>& urls);
    [[nodiscard]] quint64 importPlaylist(const QUrl& url);
    void renamePlaylist(quint64 id, const QString& name);
    void removePlaylist(quint64 id);

    [[nodiscard]] const TrackDatabase& trackDatabase() const;
    [[nodiscard]] const PlaylistDatabase& playlistDatabase() const;

Q_SIGNALS:
    void trackAdded(quint64 id, const Metadata::TrackFields& track);
    void trackModified(quint64 id, const Metadata::TrackFields& track);
    void trackRemoved(quint64 id);
    void playlistModified(quint64 id);

private:
    [[nodiscard]] Metadata::TrackFields scanFile(const QUrl& url) const;
    [[nodiscard]] QList<quint64> ensureTracksInLibrary(const QList<QUrl>& urls);
    [[nodiscard]] quint64 ensureTrackInLibrary(const QUrl& url);
    [[nodiscard]] static int filterLocalPlaylist(QList<QUrl>& result, const QUrl& playlistUrl);

    TrackDatabase m_trackDb;
    PlaylistDatabase m_playlistDb;
    QMutex m_mutex;
    std::unique_ptr<FileScanner> m_fileScanner;
};

#endif // LIBRARY_HPP
