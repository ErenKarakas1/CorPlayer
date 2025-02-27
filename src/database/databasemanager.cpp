#include "databasemanager.h"

#include "dbconnection.h"
#include "dbschema.h"

#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

void DatabaseManager::initialize(const QString& databasePath) {
    m_connectionPool = DbConnectionPool::create(databasePath);

    const DbSchema schema(DbConnection{m_connectionPool});

    if (schema.status() != DbSchema::DbStatus::Ok) {
        qWarning() << "Failed to initialize database schema";
    }
}

std::shared_ptr<DbConnectionPool> DatabaseManager::dbConnectionPool() const {
    return m_connectionPool;
}
