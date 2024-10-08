#include "sqltransaction.h"

#include <QDebug>
#include <QSqlDatabase>

SqlTransaction::SqlTransaction(const QSqlDatabase &db) : m_db(db) {
    m_db.transaction();
}

SqlTransaction::~SqlTransaction() {
    if (!m_commited) {
        qWarning() << "Rolling back transaction";

        if (!m_db.rollback()) {
            qWarning() << "Failed to rollback transaction";
        }
    }
}

bool SqlTransaction::commit() {
    if (m_commited) {
        qWarning() << "Transaction already commited";
        return false;
    }

    if (!m_db.commit()) {
        qWarning() << "Failed to commit transaction";
        return false;
    }

    m_commited = true;
    return true;
}
