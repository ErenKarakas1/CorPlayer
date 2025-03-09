#ifndef BASEDATABASE_H
#define BASEDATABASE_H

#include "database/dbconnection.h"

class BaseDatabase {
public:
	BaseDatabase() = default;
    virtual ~BaseDatabase() = default;
    void maintenance() const;
    virtual void initialize(const DbConnection& dbConnection);

protected:
    [[nodiscard]] QSqlDatabase db() const;

private:
    DbConnection m_dbConnection;
	Q_DISABLE_COPY(BaseDatabase)
};

#endif // BASEDATABASE_H
