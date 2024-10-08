#include "basedatabase.h"

#include "sqlquery.h"

void BaseDatabase::maintenance() const {
    {
        SqlQuery vacuumQuery{db(), QStringLiteral("VACUUM;")};
        if (!vacuumQuery.exec()) {
            qDebug() << "Failed to vacuum database";
        }
    }

    SqlQuery analyzeQuery{db(), QStringLiteral("ANALYZE;")};
    if (!analyzeQuery.exec()) {
        qDebug() << "Failed to analyze database";
    }
}

void BaseDatabase::initialize(const DbConnection &dbConnection) {
    m_dbConnection = dbConnection;
}

QSqlDatabase BaseDatabase::db() const {
    return m_dbConnection.db();
}
