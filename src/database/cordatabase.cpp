#include "cordatabase.h"

#include <QDebug>

CorDatabase::CorDatabase(const QString &databaseFileName, const QString &connectionName) : m_conName{connectionName} {
    auto database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);

    if (!databaseFileName.isEmpty()) {
        database.setDatabaseName(QStringLiteral("file:") + databaseFileName);
    } else {
        database.setDatabaseName(QStringLiteral("file:memdb1?mode=memory"));
    }
    // TODO look into other options for the last 2 calls
    database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_URI;"));
}

CorDatabase::CorDatabase(const CorDatabase &other, const QString &connectionName) : m_conName{connectionName} {
    QSqlDatabase::cloneDatabase(other.name(), connectionName);
}

CorDatabase::~CorDatabase() {
    qDebug() << "CorDatabase::Database connection closed" << m_conName;

    close();
    QSqlDatabase::removeDatabase(m_conName);

    qDebug() << "CorDatabase::Database connection removed" << m_conName;
}

QString CorDatabase::name() const {
    return m_conName;
}

bool CorDatabase::open() const {
    return db().open();
}

void CorDatabase::close() const {
    auto db = this->db();

    if (db.isOpen()) {
        db.close();
    }
}

bool CorDatabase::isOpen() const {
    return db().isOpen();
}

QSqlDatabase CorDatabase::db() const {
    return QSqlDatabase::database(m_conName);
}
