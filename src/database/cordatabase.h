#ifndef CORDATABASE_H
#define CORDATABASE_H

#include <QSqlDatabase>

class CorDatabase {
public:
    CorDatabase(const QString& databaseFileName, const QString& connectionName);
    CorDatabase(const CorDatabase& other, const QString& connectionName);
    ~CorDatabase();

    CorDatabase(const CorDatabase& other) = delete;
    CorDatabase(const CorDatabase&& other) = delete;

    [[nodiscard]] QString name() const;
    [[nodiscard]] bool open() const;
    void close() const;
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QSqlDatabase db() const;

private:
    QString m_conName;
};

#endif // CORDATABASE_H
