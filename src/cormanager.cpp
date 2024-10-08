#include "cormanager.h"

#include "corplayer.h"
#include "database/databasemanager.h"
#include "playlist/trackplaylist.h"
#include "trackswatchdog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QThread>

class CorManagerPrivate {
public:
    QThread m_databaseThread;
    QThread m_indexerThread;

    std::unique_ptr<TracksWatchdog> m_tracksWatchdog;

    DatabaseManager *m_dbManager = nullptr;
    CorPlayer *m_corPlayer = nullptr;
};

CorManager::CorManager(QObject *parent) : QObject(parent), cm(std::make_unique<CorManagerPrivate>()) {
    cm->m_databaseThread.start();
    cm->m_indexerThread.start();
}

CorManager::~CorManager() {
    cm->m_indexerThread.quit();
    cm->m_indexerThread.wait();

    cm->m_databaseThread.quit();
    cm->m_databaseThread.wait();
}

void CorManager::startListeningForTracks(const TrackPlaylist *playlist) {
    initTracksWatchdog();

    connect(cm->m_tracksWatchdog.get(), &TracksWatchdog::trackHasChanged, playlist, &TrackPlaylist::trackChanged);
    connect(playlist, &TrackPlaylist::addNewUrl, cm->m_tracksWatchdog.get(), &TracksWatchdog::addNewUrl);
}

CorPlayer *CorManager::corPlayer() const {
    return cm->m_corPlayer;
}

TracksWatchdog *CorManager::tracksWatchdog() const {
    return cm->m_tracksWatchdog.get();
}

void CorManager::setDatabaseManager(DatabaseManager *dbManager) {
    cm->m_dbManager = dbManager;

    const auto dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const bool success = QDir().mkpath(dbPath);

    if (!success) {
        qWarning() << "Failed to create directory " << dbPath;
    }

    const auto dbFile = dbPath + QStringLiteral("/corplayer.db");
    cm->m_dbManager->initialize(dbFile);

    Q_EMIT databaseManagerChanged();
}

void CorManager::setCorPlayer(CorPlayer *corPlayer) {
    if (cm->m_corPlayer == corPlayer) return;

    cm->m_corPlayer = corPlayer;
    Q_EMIT corPlayerChanged();
}

void CorManager::initTracksWatchdog() {
    if (cm->m_tracksWatchdog) return;

    cm->m_tracksWatchdog = std::make_unique<TracksWatchdog>();
    cm->m_tracksWatchdog->moveToThread(&cm->m_databaseThread);
    QMetaObject::invokeMethod(cm->m_tracksWatchdog.get(), "initDatabase", Qt::QueuedConnection,
                              Q_ARG(std::shared_ptr<DbConnectionPool>, cm->m_dbManager->dbConnectionPool()));

    Q_EMIT tracksWatchdogChanged();
}

#include "moc_cormanager.cpp"
