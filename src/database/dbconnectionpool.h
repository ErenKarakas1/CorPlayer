#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H

#include <QString>
#include <QThreadStorage>

#include <memory>

class QSqlDatabase;
class CorDatabase;

class DbConnectionPool {
    Q_DISABLE_COPY_MOVE(DbConnectionPool)

public:
    explicit DbConnectionPool(QString databaseName);
    static std::shared_ptr<DbConnectionPool> create(const QString& databaseName);
    [[nodiscard]] bool hasConnection() const;

private:
    friend class DbConnection;

    QString m_databaseName;
    QThreadStorage<CorDatabase*> m_threadConnections;

    bool createConnection();

    [[nodiscard]] CorDatabase* acquire() const;
    void release();
};

#endif // DBCONNECTIONPOOL_H
