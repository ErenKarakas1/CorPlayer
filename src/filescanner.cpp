#include "filescanner.h"

#include <QFileInfo>
#include <QMimeDatabase>

class FileScannerPrivate {
public:
    QMimeDatabase m_mimeDB;
};

FileScanner::FileScanner() : fs(std::make_unique<FileScannerPrivate>()) {}

FileScanner::~FileScanner() = default;

MetadataFields::TrackMetadataField FileScanner::scanFile(const QUrl &file) {
    const QFileInfo fileInfo(file.toLocalFile());
    return FileScanner::scanFile(file, fileInfo);
}

MetadataFields::TrackMetadataField FileScanner::scanFile(const QUrl &file, const QFileInfo &fileInfo) {
    MetadataFields::TrackMetadataField newTrack;

    // if (!file.scheme().isEmpty()) {
    //     qDebug() << "FileScanner::scanFile found not empty";
    //     return newTrack;
    // }

    newTrack[MetadataFields::ResourceRole] = file;
    newTrack[MetadataFields::ElementTypeRole] = PlayerUtils::Track;

    Q_UNUSED(file);
    Q_UNUSED(fileInfo);

    return newTrack;
}
