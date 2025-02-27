#ifndef TRACKTAGS_H
#define TRACKTAGS_H

#include "metadata.hpp"

class TrackTagsPrivate;

class TrackTags {
public:
    explicit TrackTags(const QString& fileName);
    ~TrackTags();

    [[nodiscard]] QString fileName() const;
    [[nodiscard]] QByteArray coverImage() const;
    [[nodiscard]] const QMultiMap<Metadata::Fields, QVariant>& fieldMapping() const;
    [[nodiscard]] QVariant value(Metadata::Fields field) const;
    void add(Metadata::Fields field, const QVariant& value) const;
    void addCoverImage(const QByteArray& image) const;

private:
    std::unique_ptr<TrackTagsPrivate> tt;
};

#endif // TRACKTAGS_H
