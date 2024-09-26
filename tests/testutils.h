#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <QDir>
#include <QObject>
#include <QTemporaryFile>

class TemporaryFile : public QTemporaryFile {
public:
    explicit TemporaryFile(const QString &fileName, QObject *parent = nullptr);
};

#endif // TESTUTILS_H
