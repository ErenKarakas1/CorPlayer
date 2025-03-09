#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "database/dbconnectionpool.h"

#include <memory>

class DatabaseManager {
public:
    static DatabaseManager& instance();
    void initialize(const QString& databasePath);

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    [[nodiscard]] std::shared_ptr<DbConnectionPool> dbConnectionPool() const;

private:
    DatabaseManager() = default;
    ~DatabaseManager() = default;

    std::shared_ptr<DbConnectionPool> m_connectionPool;
};

#endif // DATABASEMANAGER_H
