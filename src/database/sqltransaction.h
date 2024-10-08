#ifndef SQLTRANSACTION_H
#define SQLTRANSACTION_H

#include <QSqlDatabase>

class SqlTransaction {
public:
    explicit SqlTransaction(const QSqlDatabase &db);
    ~SqlTransaction();
    [[nodiscard]] bool commit();

    SqlTransaction(const SqlTransaction &other) = delete;
    SqlTransaction &operator=(const SqlTransaction &other) = delete;

private:
    QSqlDatabase m_db;
    bool m_commited{false};
};

#endif // SQLTRANSACTION_H
