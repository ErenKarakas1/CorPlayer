#include "filescanner.h"

#include "taglib/tagreader.h"
#include "taglib/tracktags.h"

#include <QFileInfo>
#include <QLatin1StringView>
#include <QMimeDatabase>

class FileScannerPrivate {
public:
    QMimeDatabase m_mimeDB;
    TagReader m_tagReader;
};

FileScanner::FileScanner() : fs(std::make_unique<FileScannerPrivate>()) {}

FileScanner::~FileScanner() = default;

MetadataFields::TrackMetadataField FileScanner::scanFile(const QUrl &file) {
    const QFileInfo fileInfo(file.toLocalFile());
    return FileScanner::scanFile(file, fileInfo);
}

MetadataFields::TrackMetadataField FileScanner::scanFile(const QUrl &file, const QFileInfo &fileInfo) {
    MetadataFields::TrackMetadataField newTrack;

    if (!file.isLocalFile()) {
        return newTrack;
    }

    newTrack[MetadataFields::FileModificationTime] = fileInfo.metadataChangeTime();
    newTrack[MetadataFields::ResourceRole] = file;
    newTrack[MetadataFields::ElementTypeRole] = PlayerUtils::Track;
    newTrack[MetadataFields::DurationRole] = QTime::fromMSecsSinceStartOfDay(1);

    const auto &localFile = file.toLocalFile();
    const auto &mimeType = fs->m_mimeDB.mimeTypeForFile(localFile);

    if (!mimeType.name().startsWith(QLatin1StringView("audio/"))) {
        return newTrack;
    }

    TrackTags tags = TrackTags(localFile);
    fs->m_tagReader.readMetadata(localFile, tags);

    auto rangeBegin = tags.roleMapping().constKeyValueBegin();

    while (rangeBegin != tags.roleMapping().constKeyValueEnd()) {
        const auto &[role, value] = *rangeBegin;
        newTrack[role] = value;
        ++rangeBegin;
    }

    if (tryExtractEmbeddedCoverImage(localFile)) {
        newTrack[MetadataFields::HasEmbeddedCover] = true;
        newTrack[MetadataFields::ImageUrlRole] = QUrl(QLatin1StringView("image://cover/") + localFile);
    } else {
        newTrack[MetadataFields::HasEmbeddedCover] = false;
    }

    return newTrack;
}

bool FileScanner::tryExtractEmbeddedCoverImage(const QString &localFile) {
    TrackTags tags = TrackTags(localFile);
    fs->m_tagReader.extractCoverImage(localFile, tags);

    return !tags.coverImage().isEmpty();
}
