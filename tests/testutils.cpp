#include "testutils.h"

#include <QDir>
#include <QTemporaryFile>

TemporaryFile::TemporaryFile(const QString &fileName, QObject *parent) : QTemporaryFile{parent} {
    setFileTemplate(QDir::tempPath() + QStringLiteral("/temp-XXXXXX"));
    open();

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    write(file.readAll());

    QTemporaryFile::reset();
}
