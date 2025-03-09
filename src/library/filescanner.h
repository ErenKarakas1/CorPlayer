#ifndef FILESCANNER_H
#define FILESCANNER_H

#include "metadata.hpp"

#include <QFileInfo>

class QUrl;
class FileScannerPrivate;

class FileScanner {
public:
    FileScanner();
    ~FileScanner();

    [[nodiscard]] Metadata::TrackFields scanFile(const QUrl& file) const;
    [[nodiscard]] QList<Metadata::TrackFields> scanFiles(const QList<QUrl>& files) const;

private:
    std::unique_ptr<FileScannerPrivate> fs;
};

#endif // FILESCANNER_H
