#include "dbconnection.h"

#include "cordatabase.h"

#include <QDebug>
#include <QSqlDatabase>
#include <utility>

DbConnection::DbConnection() : m_connectionPool{nullptr} {}

DbConnection::DbConnection(std::shared_ptr<DbConnectionPool> dbConnectionPool)
    : m_connectionPool(std::move(dbConnectionPool)) {}

DbConnection::~DbConnection() {
    if (isValid() && m_connectionPool->hasConnection()) {
        m_connectionPool->release();
    }
}

bool DbConnection::isValid() const {
    return m_connectionPool != nullptr;
}

QSqlDatabase DbConnection::db() const {
    if (!isValid()) {
        qWarning() << "No pool assigned";
        return {};
    }

    if (!m_connectionPool->hasConnection()) {
        m_connectionPool->createConnection();
    }

    const CorDatabase *const dbConnection = m_connectionPool->acquire();

    if (dbConnection == nullptr) {
        qWarning() << "Could not acquire connection";
        return {};
    }

    if (!dbConnection->isOpen() && !dbConnection->db().open()) {
        qWarning() << "Failed to open database connection";
        return {};
    }

    return dbConnection->db();
}
