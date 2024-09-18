#ifndef FILESCANNER_H
#define FILESCANNER_H

#include "metadatafields.h"

class QUrl;
class QFileInfo;
class FileScannerPrivate;

class FileScanner {
public:
    FileScanner();
    ~FileScanner();
    MetadataFields::TrackMetadataField scanFile(const QUrl &file);
    MetadataFields::TrackMetadataField scanFile(const QUrl &file, const QFileInfo &fileInfo);

private:
    bool tryExtractEmbeddedCoverImage(const QString &localFile);
    std::unique_ptr<FileScannerPrivate> fs;
};

#endif // FILESCANNER_H
