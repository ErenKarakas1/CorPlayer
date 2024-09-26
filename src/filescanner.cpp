#include "filescanner.h"

#include "playerutils.h"
#include "taglib/tagreader.h"
#include "taglib/tracktags.h"

#include <QLatin1StringView>
#include <QMimeDatabase>

#include <algorithm>

class FileScannerPrivate {
public:
    QMimeDatabase m_mimeDB;
    TagReader m_tagReader;
};

FileScanner::FileScanner() : fs(std::make_unique<FileScannerPrivate>()) {}

FileScanner::~FileScanner() = default;

MetadataFields::TrackMetadataField FileScanner::scanFile(const QUrl &file) const {
    const QFileInfo fileInfo(file.toLocalFile());
    return scanFile(file, fileInfo);
}

MetadataFields::TrackMetadataField FileScanner::scanFile(const QUrl &file, const QFileInfo &fileInfo) const {
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

    TrackTags tags(localFile);
    fs->m_tagReader.readMetadata(localFile, tags);

    const auto roleMapping = tags.roleMapping();
    auto rangeBegin = roleMapping.constKeyValueBegin();
    QVariant value;

    while (rangeBegin != roleMapping.constKeyValueEnd()) {
        const auto key = rangeBegin->first;
        const auto duplicateRangeEnd =
            std::ranges::find_if(rangeBegin, roleMapping.constKeyValueEnd(), [key](const auto &pair) {
                return pair.first != key;
            });

        const auto duplicateCount = std::ranges::distance(rangeBegin, duplicateRangeEnd);

        if (duplicateCount > 1) {
            QStringList values;
            values.reserve(static_cast<int>(duplicateCount));

            std::ranges::for_each(rangeBegin, duplicateRangeEnd, [&values](const auto &pair) {
                values.append(pair.second.toString());
            });

            value = QLocale().createSeparatedList(values);
        } else {
            value = rangeBegin->second;
        }

        if (key == MetadataFields::DurationRole) {
            newTrack.insert(key,
                            QTime::fromMSecsSinceStartOfDay(static_cast<int>(1000 * rangeBegin->second.toDouble())));
        } else {
            newTrack.insert(key, value);
        }

        rangeBegin = duplicateRangeEnd;
    }

    if (newTrack[MetadataFields::HasEmbeddedCover].toBool()) {
        newTrack[MetadataFields::ImageUrlRole] = QUrl(QLatin1StringView("image://cover/") + localFile);
    }

    newTrack[MetadataFields::HashRole] = newTrack.generateHash();

    return newTrack;
}
