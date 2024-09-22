#include "sqlquery.h"

SqlQuery::SqlQuery(const QSqlDatabase &db, const QString &statement) : QSqlQuery(db) {
    setForwardOnly(true);
    prepare(statement);
}

void SqlQuery::bindValue(const QString &placeholder, const QVariant &val) {
    m_boundValues.insert(placeholder, val);
    QSqlQuery::bindValue(placeholder, val);
}

void SqlQuery::bindStringValue(const QString &placeholder, const QString &value) {
    bindValue(placeholder, value.isNull() ? QStringLiteral("") : value);
}

void SqlQuery::bindUrlValue(const QString &placeholder, const QUrl &value) {
    bindValue(placeholder, value.isValid() ? value.toString(QUrl::FullyEncoded) : QStringLiteral(""));
}

bool SqlQuery::exec() {
    const bool success = QSqlQuery::exec();
    m_lastQuery = executedQuery();

    const QList<QString> keys = m_boundValues.keys();

    for (const QString &key : keys) {
        m_lastQuery.replace(key, m_boundValues.value(key).toString());
    }

    m_boundValues.clear();

    return success;
}

QString SqlQuery::lastQuery() const {
    return m_lastQuery;
}
