#ifndef FILESCANNER_H
#define FILESCANNER_H

#include "metadatafields.h"

#include <QFileInfo>

class QUrl;
class FileScannerPrivate;

class FileScanner {
public:
    FileScanner();
    ~FileScanner();
    [[nodiscard]] MetadataFields::TrackMetadataField scanFile(const QUrl &file) const;
    [[nodiscard]] MetadataFields::TrackMetadataField scanFile(const QUrl &file, const QFileInfo &fileInfo) const;

private:
    [[nodiscard]] bool tryExtractEmbeddedCoverImage(const QString &localFile) const;
    std::unique_ptr<FileScannerPrivate> fs;
};

#endif // FILESCANNER_H
