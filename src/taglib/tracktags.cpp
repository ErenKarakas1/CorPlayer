#include "taglib/tracktags.h"

class TrackTagsPrivate {
public:
    QString m_fileName;
    QByteArray m_coverImage;
    QMultiMap<Metadata::Fields, QVariant> m_fieldMapping;
};

TrackTags::TrackTags(const QString& fileName) : tt(std::make_unique<TrackTagsPrivate>()) {
    tt->m_fileName = fileName;
}

TrackTags::~TrackTags() = default;

QString TrackTags::fileName() const {
    return tt->m_fileName;
}

QByteArray TrackTags::coverImage() const {
    return tt->m_coverImage;
}

const QMultiMap<Metadata::Fields, QVariant>& TrackTags::fieldMapping() const {
    return tt->m_fieldMapping;
}

QVariant TrackTags::value(const Metadata::Fields field) const {
    return tt->m_fieldMapping.value(field);
}

void TrackTags::add(const Metadata::Fields field, const QVariant& value) const {
    if (tt->m_fieldMapping.values(field).contains(value)) return;

    tt->m_fieldMapping.insert(field, value);
}

void TrackTags::addCoverImage(const QByteArray& image) const {
    if (image.isEmpty()) {
        add(Metadata::Fields::HasEmbeddedCover, false);
    } else {
        add(Metadata::Fields::HasEmbeddedCover, true);
        tt->m_coverImage = image;
    }
}
