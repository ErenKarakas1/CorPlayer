#ifndef DBSCHEMA_H
#define DBSCHEMA_H

#include "dbconnection.h"

#include <QObject>

class DbSchema : QObject {
    Q_OBJECT

public:
    enum class DbStatus { Ok, DatabaseError, BrokenSchemaError, ConnectionError };

    enum class SchemaStatus { Latest, SuccessfulUpgrade, FailedUpgrade };

    Q_PROPERTY(DbStatus status READ status WRITE setStatus NOTIFY statusChanged)

    explicit DbSchema(const DbConnection& dbConnection, QObject* parent = nullptr);

    [[nodiscard]] DbStatus status() const;
    void setStatus(DbStatus status);

    // [[nodiscard]] int currentVersion() const;
    // [[nodiscard]] int previousVersion() const;
    // [[nodiscard]] SchemaStatus upgradeSchema();

Q_SIGNALS:
    void statusChanged(DbStatus status);
    void schemaChanged(int newVersion);

private:
    bool createSchema(const QSqlDatabase& db);

    DbStatus m_status;
};

#endif // DBSCHEMA_H
