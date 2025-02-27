#include "filescanner.h"

#include "playerutils.hpp"
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

Metadata::TrackFields FileScanner::scanFile(const QUrl& file) const {
    const QFileInfo fileInfo(file.toLocalFile());
    return scanFile(file, fileInfo);
}

Metadata::TrackFields FileScanner::scanFile(const QUrl& file, const QFileInfo& fileInfo) const {
    Metadata::TrackFields newTrack;

    if (!file.isLocalFile()) {
        return newTrack;
    }

    newTrack.insert(Metadata::Fields::DateModified, fileInfo.metadataChangeTime());
    newTrack.insert(Metadata::Fields::ResourceUrl, file);
    newTrack.insert(Metadata::Fields::ElementType, PlayerUtils::Track);
    newTrack.insert(Metadata::Fields::Duration, QTime::fromMSecsSinceStartOfDay(1));

    const QString& localFile = file.toLocalFile();
    const auto& mimeType = fs->m_mimeDB.mimeTypeForFile(localFile);

    if (!mimeType.name().startsWith(QLatin1StringView("audio/"))) {
        return newTrack;
    }

    TrackTags tags(localFile);
    fs->m_tagReader.readMetadata(localFile, tags);

    const auto roleMapping = tags.fieldMapping();
    auto rangeBegin = roleMapping.constKeyValueBegin();
    QVariant value;

    while (rangeBegin != roleMapping.constKeyValueEnd()) {
        const auto key = rangeBegin->first;
        const auto duplicateRangeEnd =
            std::ranges::find_if(rangeBegin, roleMapping.constKeyValueEnd(), [key](const auto& pair) {
                return pair.first != key;
            });

        const auto duplicateCount = std::ranges::distance(rangeBegin, duplicateRangeEnd);

        if (duplicateCount > 1) {
            QStringList values;
            values.reserve(static_cast<int>(duplicateCount));

            std::ranges::for_each(rangeBegin, duplicateRangeEnd, [&values](const auto& pair) {
                values.append(pair.second.toString());
            });

            value = QLocale().createSeparatedList(values);
        } else {
            value = rangeBegin->second;
        }

        if (key == Metadata::Fields::Duration) {
            newTrack.insert(key,
                            QTime::fromMSecsSinceStartOfDay(static_cast<int>(1000 * rangeBegin->second.toDouble())));
        } else {
            newTrack.insert(key, value);
        }

        rangeBegin = duplicateRangeEnd;
    }

    if (newTrack.get(Metadata::Fields::HasEmbeddedCover).toBool()) {
        newTrack.insert(Metadata::Fields::CoverImage, QUrl(QLatin1StringView("image://cover/") + localFile));
    }

    newTrack.insert(Metadata::Fields::Hash, newTrack.generateHash());

    return newTrack;
}
