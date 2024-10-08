#ifndef CORMANAGER_H
#define CORMANAGER_H

#include <QQmlEngine>

class TracksWatchdog;
class DbConnectionPool;
class TrackPlaylist;
class CorPlayer;
class DatabaseManager;
class CorManagerPrivate;

class CorManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(DatabaseManager *databaseManager WRITE setDatabaseManager NOTIFY databaseManagerChanged)
    Q_PROPERTY(CorPlayer *corPlayer READ corPlayer WRITE setCorPlayer NOTIFY corPlayerChanged)
    Q_PROPERTY(TracksWatchdog *tracksWatchdog READ tracksWatchdog NOTIFY tracksWatchdogChanged)

public:
    explicit CorManager(QObject *parent = nullptr);
    ~CorManager() override;
    void startListeningForTracks(const TrackPlaylist *playlist);
    [[nodiscard]] CorPlayer *corPlayer() const;
    [[nodiscard]] TracksWatchdog *tracksWatchdog() const;

Q_SIGNALS:
    void databaseManagerChanged();
    void corPlayerChanged();
    void tracksWatchdogChanged();

public Q_SLOTS:
    void setDatabaseManager(DatabaseManager *dbManager);
    void setCorPlayer(CorPlayer *corPlayer);

private:
    void initTracksWatchdog();
    std::unique_ptr<CorManagerPrivate> cm;
};

#endif // CORMANAGER_H
