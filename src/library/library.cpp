#include "library/library.hpp"

#include "library/filescanner.h"
#include "library/playlistparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThreadPool>

Library::Library(QObject* parent) : QObject(parent), m_fileScanner(std::make_unique<FileScanner>()) {}

Library::~Library() = default;

void Library::initialize(const std::shared_ptr<DbConnectionPool>& pool) {
    m_trackDb.initialize(DbConnection{pool});
    m_playlistDb.initialize(DbConnection{pool});
}

QList<quint64> Library::addTracksFromUrls(const QList<QUrl>& urls) {
    QList<quint64> trackIds;
    trackIds.reserve(urls.size());

    for (const QUrl& url : urls) {
        const quint64 trackId = addTrackFromUrl(url);
        if (trackId != 0) {
            trackIds.append(trackId);
        }
    }

    return trackIds;
}

quint64 Library::addTrackFromUrl(const QUrl& url) {
    if (!url.isLocalFile()) return 0;

    quint64 trackId = m_trackDb.fetchTrackIdFromFileName(url);
    if (trackId != 0) return trackId;

    const auto trackFields = scanFile(url);
    if (!trackFields.isValid()) return 0;

    auto tracks = QList{trackFields};
    if (!m_trackDb.insertTracks(tracks)) return 0;

    trackId = m_trackDb.fetchTrackIdFromFileName(url);
    if (trackId != 0) {
        Q_EMIT trackAdded(trackId, trackFields);
    }

    return trackId;
}

Metadata::TrackFields Library::getTrackById(const quint64 id) const {
    return m_trackDb.fetchTrackFromId(id);
}

void Library::updateTrack(const Metadata::TrackFields& track) {
    auto tracks = QList{track};
    if (m_trackDb.updateTracks(tracks)) {
        Q_EMIT trackModified(track.get(Metadata::Fields::DatabaseId).toULongLong(), track);
    }
}

void Library::removeTrack(const quint64 id) {
    if (m_trackDb.deleteTrack(id)) {
        Q_EMIT trackRemoved(id);
    }
}

quint64 Library::createPlaylistFromUrls(const QString& name, const QList<QUrl>& urls) {
    QList<quint64> trackIds = ensureTracksInLibrary(urls);

    if (trackIds.isEmpty()) return 0;

    Metadata::PlaylistRecord record;
    record.name = name;
    record.trackIds = std::move(trackIds);

    if (!m_playlistDb.savePlaylist(record)) return 0;

    const auto playlist = m_playlistDb.getPlaylist(name);
    if (playlist.id != 0) {
        Q_EMIT playlistModified(playlist.id);
        return playlist.id;
    }

    return 0;
}

quint64 Library::importPlaylist(const QUrl& url) {
    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) return 0;

    auto urls = PlaylistParser::fromPlaylist(url, file.readAll());
    if (urls.isEmpty()) return 0;

    const int filtered = filterLocalPlaylist(urls, url);
    if (filtered != 0) {
        // TODO
        qDebug() << "Some files in the playlist could not be found in the file system";
    }

    return createPlaylistFromUrls(QFileInfo(file).baseName(), urls);
}

void Library::renamePlaylist(const quint64 id, const QString& name) {
    auto playlist = m_playlistDb.getPlaylist(id);
    if (playlist.id == 0) return;

    playlist.name = name;
    if (m_playlistDb.updatePlaylist(playlist)) {
        Q_EMIT playlistModified(id);
    }
}

void Library::removePlaylist(const quint64 id) {
    if (m_playlistDb.removePlaylist(id)) {
        Q_EMIT playlistModified(id);
    }
}

const TrackDatabase& Library::trackDatabase() const {
    return m_trackDb;
}

const PlaylistDatabase& Library::playlistDatabase() const {
    return m_playlistDb;
}

Metadata::TrackFields Library::scanFile(const QUrl& url) const {
    return m_fileScanner->scanFile(url);
}

QList<quint64> Library::ensureTracksInLibrary(const QList<QUrl>& urls) {
    QHash<QUrl, quint64> trackIdLookup = m_trackDb.fetchTrackIdsFromFileNames(urls);

    QList<QUrl> urlsToScan;
    urlsToScan.reserve(trackIdLookup.size());

    for (const QUrl& url : urls) {
        // can trackIdLookup[url] == 0 be true?
        if (!trackIdLookup.contains(url) || trackIdLookup[url] == 0) {
            urlsToScan.append(url);
        }
    }

    if (!urlsToScan.isEmpty()) {
        QList<Metadata::TrackFields> scannedTracks;

        QThreadPool pool;
        const int ideal = QThread::idealThreadCount();
        qInfo() << "Scanning metadata for " << urlsToScan.size() << " files using " << ideal << " threads";
        pool.setMaxThreadCount(ideal);

        constexpr int chunkSize = 100; // TODO: to be determined

        for (int i = 0; i < urlsToScan.size(); i += chunkSize) {
            const int end = qMin(i + chunkSize, static_cast<int>(urlsToScan.size()));

            pool.start([this, &urlsToScan, i, end, &scannedTracks] {
                QList<Metadata::TrackFields> tracks;
                tracks.reserve(end - i);

                for (int j = i; j < end; ++j) {
                    const auto trackFields = scanFile(urlsToScan[j]);
                    if (trackFields.isValid()) {
                        tracks.append(trackFields);
                    }
                }

                if (!tracks.isEmpty()) {
                    const QMutexLocker locker(&m_mutex);
                    scannedTracks.append(std::move(tracks));
                }
            });
        }

        pool.waitForDone();

        if (!scannedTracks.isEmpty() && m_trackDb.insertTracks(scannedTracks)) {
            for (const auto& track : scannedTracks) {
                const QUrl url = track.get(Metadata::Fields::ResourceUrl).toUrl();
                const quint64 trackId = track.get(Metadata::Fields::DatabaseId).toULongLong();

                if (trackId != 0) {
                    trackIdLookup[url] = trackId;
                    Q_EMIT trackAdded(trackId, track);
                }
            }
        }
    }

    QList<quint64> validTrackIds;
    validTrackIds.reserve(urls.size());

    for (const QUrl& url : urls) {
        const quint64 trackId = trackIdLookup.value(url, 0);
        if (trackId != 0) {
            validTrackIds.append(trackId);
        }
    }

    return validTrackIds;
}

quint64 Library::ensureTrackInLibrary(const QUrl& url) {
    return addTrackFromUrl(url);
}

int Library::filterLocalPlaylist(QList<QUrl>& result, const QUrl& playlistUrl) {
    int filtered = 0;

    for (auto iterator = result.begin(); iterator != result.end();) {
        bool exists = true;

        QUrl& url = *iterator;
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

// void TrackPlaylistProxyModel::loadLocalFile(Metadata::EntryFieldsList& newTracks, QSet<QString>& processedFiles,
//                                             const QFileInfo& fileInfo) {
//     // protection against recursion
//     const auto canonicalFilePath = fileInfo.canonicalFilePath();
//     if (processedFiles.contains(canonicalFilePath)) return;
//
//     processedFiles.insert(canonicalFilePath);
//
//     const auto fileUrl = QUrl::fromLocalFile(fileInfo.filePath());
//     const auto mimeType = tpp->m_mimeDB.mimeTypeForUrl(fileUrl);
//     if (fileInfo.isDir()) {
//         if (fileInfo.isSymLink()) return;
//         loadLocalDirectory(newTracks, processedFiles, fileUrl);
//     } else {
//         if (!mimeType.name().startsWith(QLatin1String("audio/"))) return;
//
//         if (PlayerUtils::isPlaylist(mimeType)) {
//             QFile file(fileInfo.filePath());
//             if (!file.open(QIODevice::ReadOnly)) {
//                 tpp->m_partiallyLoaded = true;
//                 return;
//             }
//             loadLocalPlaylist(newTracks, processedFiles, fileUrl, file.readAll());
//             return;
//         }
//         auto entry = Metadata::TrackFields();
//         entry.insert(Metadata::Fields::ElementType, PlayerUtils::FileName);
//         entry.insert(Metadata::Fields::ResourceUrl, fileUrl);
//         newTracks.emplace_back(Metadata::EntryFields{.trackFields = entry, .title = {}, .url = {}});
//     }
// }

// void TrackPlaylistProxyModel::loadLocalPlaylist(Metadata::EntryFieldsList& newTracks, QSet<QString>& processedFiles,
//                                                 const QUrl& fileName, const QByteArray& fileContent) {
//     QList<QUrl> listOfUrls = PlaylistParser::fromPlaylist(fileName, fileContent);
//
//     const int filtered = filterLocalPlaylist(listOfUrls, fileName);
//     if (filtered != 0) {
//         tpp->m_partiallyLoaded = true;
//     }
//
//     for (const QUrl& oneUrl : std::as_const(listOfUrls)) {
//         if (oneUrl.isLocalFile()) {
//             const QFileInfo fileInfo(oneUrl.toLocalFile());
//             loadLocalFile(newTracks, processedFiles, fileInfo);
//         } else {
//             auto entry = Metadata::TrackFields();
//             entry.insert(Metadata::Fields::ElementType, PlayerUtils::FileName);
//             entry.insert(Metadata::Fields::ResourceUrl, oneUrl);
//             newTracks.emplace_back(Metadata::EntryFields{entry, {}, {}});
//         }
//     }
// }
//
// void TrackPlaylistProxyModel::loadLocalDirectory(Metadata::EntryFieldsList& newTracks, QSet<QString>& processedFiles,
//                                                  const QUrl& dirName) {
//     const QDir dirInfo(dirName.toLocalFile());
//     const auto fileInfoList =
//         dirInfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Files | QDir::Dirs, QDir::Name);
//
//     for (const auto& fileInfo : fileInfoList) {
//         loadLocalFile(newTracks, processedFiles, fileInfo);
//     }
// }
