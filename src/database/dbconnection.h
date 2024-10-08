#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include "dbconnectionpool.h"

class DbConnection {
public:
    DbConnection();
    explicit DbConnection(std::shared_ptr<DbConnectionPool> dbConnectionPool);
    ~DbConnection();

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] QSqlDatabase db() const;

private:
    std::shared_ptr<DbConnectionPool> m_connectionPool;
};

#endif // DBCONNECTION_H
