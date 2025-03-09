#include "database/sqlquery.h"

SqlQuery::SqlQuery(const QSqlDatabase& db, const QString& statement) : QSqlQuery(db) {
    setForwardOnly(true);
    prepare(statement);
}

void SqlQuery::bindValue(const QString& placeholder, const QVariant& value) {
    m_boundValues.insert(placeholder, value);
    QSqlQuery::bindValue(placeholder, value);
}

void SqlQuery::bindStringValue(const QString& placeholder, const QString& value) {
    bindValue(placeholder, value.isNull() ? QLatin1String("") : value);
}

void SqlQuery::bindBoolValue(const QString& placeholder, const bool value) {
    bindValue(placeholder, value ? 1 : 0);
}

bool SqlQuery::exec() {
    const bool success = QSqlQuery::exec();
    m_lastQuery = executedQuery();

    const QList<QString> keys = m_boundValues.keys();

    for (const QString& key : keys) {
        m_lastQuery.replace(key, m_boundValues.value(key).toString());
    }

    m_boundValues.clear();

    return success;
}

QString SqlQuery::lastQuery() const {
    return m_lastQuery;
}
