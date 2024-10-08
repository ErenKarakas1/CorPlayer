#include "dbconnectionpool.h"

#include "cordatabase.h"
#include "sqlquery.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QThread>

DbConnectionPool::DbConnectionPool(QString databaseName) : m_databaseName{std::move(databaseName)} {}

std::shared_ptr<DbConnectionPool> DbConnectionPool::create(const QString &databaseName) {
    return std::make_shared<DbConnectionPool>(databaseName);
}

bool DbConnectionPool::hasConnection() const {
    return m_threadConnections.hasLocalData();
}

bool DbConnectionPool::createConnection() {
    if (hasConnection()) {
        qWarning() << "Connection already exists: " << m_threadConnections.localData()->name();
        return false;
    }

    const QString connectionName =
        QStringLiteral("Connection-%1").arg(QString::number(QRandomGenerator::global()->generate()));

    auto db = std::make_unique<CorDatabase>(m_databaseName, connectionName);

    if (!db->open()) {
        qWarning() << "Failed to open database connection";
        return false;
    }

    const QString statement = QStringLiteral("PRAGMA foreign_keys = ON;");
    SqlQuery query{db->db(), statement};

    if (!query.exec()) {
        qWarning() << "Failed to enable foreign keys for database";
        return false;
    }

    m_threadConnections.setLocalData(db.release());

    return true;
}

CorDatabase *DbConnectionPool::acquire() const {
    if (!hasConnection()) {
        qWarning() << "DbConnectionPool::acquire: No connection available";
    }

    return m_threadConnections.localData();
}

void DbConnectionPool::release() {
    if (!hasConnection()) {
        qWarning() << "DbConnectionPool::release: No connection available";
    }

    m_threadConnections.setLocalData(nullptr);
}
