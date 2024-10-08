#ifndef BASEDATABASE_H
#define BASEDATABASE_H

#include "dbconnection.h"

class BaseDatabase {
public:
    virtual ~BaseDatabase() = default;
    void maintenance() const;
    virtual void initialize(const DbConnection &dbConnection);

protected:
    [[nodiscard]] QSqlDatabase db() const;

private:
    DbConnection m_dbConnection;
};

#endif // BASEDATABASE_H
