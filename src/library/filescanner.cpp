#include "library/filescanner.h"

#include "playerutils.hpp"
#include "taglib/tagreader.h"
#include "taglib/tracktags.h"

#include <QMimeDatabase>

#include <algorithm>

class FileScannerPrivate {
public:
    QMimeDatabase m_mimeDb;
    TagReader m_tagReader;
};

FileScanner::FileScanner() : fs(std::make_unique<FileScannerPrivate>()) {}

FileScanner::~FileScanner() = default;

Metadata::TrackFields FileScanner::scanFile(const QUrl& file) const {
    Metadata::TrackFields newTrack{};

    if (!file.isLocalFile()) return newTrack;

    const QString filePath = file.toLocalFile();
    const QFileInfo fileInfo(filePath);

    if (!fs->m_mimeDb.mimeTypeForFile(filePath).name().startsWith("audio/")) return newTrack;

    newTrack.insert(Metadata::Fields::DateModified, fileInfo.metadataChangeTime());
    newTrack.insert(Metadata::Fields::ResourceUrl, file);
    newTrack.insert(Metadata::Fields::ElementType, PlayerUtils::Track);
    newTrack.insert(Metadata::Fields::Duration, QTime::fromMSecsSinceStartOfDay(1));

    TrackTags tags(filePath);
    fs->m_tagReader.readMetadata(filePath, tags);

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
        newTrack.insert(Metadata::Fields::CoverImage, QUrl("image://cover/" + filePath));
    }

    newTrack.insert(Metadata::Fields::Hash, newTrack.generateHash());

    return newTrack;
}

QList<Metadata::TrackFields> FileScanner::scanFiles(const QList<QUrl>& files) const {
    QList<Metadata::TrackFields> result;
    result.reserve(files.size());

    for (const QUrl& file : files) {
        if (auto track = scanFile(file); track.isValid()) {
            result.append(std::move(track));
        }
    }

    return result;
}
