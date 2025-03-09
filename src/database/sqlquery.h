#ifndef SQLQUERY_H
#define SQLQUERY_H

#include <QSqlQuery>

class SqlQuery : public QSqlQuery {
public:
    explicit SqlQuery(const QSqlDatabase& db, const QString& statement);

    void bindValue(const QString& placeholder, const QVariant& value);
    void bindStringValue(const QString& placeholder, const QString& value);
    void bindBoolValue(const QString& placeholder, bool value);

    [[nodiscard]] bool exec();
    [[nodiscard]] QString lastQuery() const;

private:
    QString m_lastQuery;
    QMap<QString, QVariant> m_boundValues;
};

#endif // SQLQUERY_H
