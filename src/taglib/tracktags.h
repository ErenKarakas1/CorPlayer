#ifndef TRACKTAGS_H
#define TRACKTAGS_H

#include "metadatafields.h"

class TrackTagsPrivate;

class TrackTags {
public:
    explicit TrackTags(const QString &fileName);
    ~TrackTags();

    [[nodiscard]] QString fileName() const;
    [[nodiscard]] QByteArray coverImage() const;
    [[nodiscard]] const QMultiMap<MetadataFields::ColumnsRoles, QVariant> &roleMapping() const;
    [[nodiscard]] QVariant value(MetadataFields::ColumnsRoles role) const;
    void add(MetadataFields::ColumnsRoles role, const QVariant &value) const;
    void addCoverImage(const QByteArray &image) const;

private:
    std::unique_ptr<TrackTagsPrivate> tt;
};

#endif // TRACKTAGS_H
