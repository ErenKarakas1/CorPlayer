#include "tracktags.h"

class TrackTagsPrivate {
public:
    QString m_fileName;
    QByteArray m_coverImage;
    QMultiMap<MetadataFields::ColumnsRoles, QVariant> m_roleMapping;
};

TrackTags::TrackTags(const QString &fileName) : tt(std::make_unique<TrackTagsPrivate>()) {
    tt->m_fileName = fileName;
}

TrackTags::~TrackTags() = default;

QString TrackTags::fileName() const {
    return tt->m_fileName;
}

QByteArray TrackTags::coverImage() const {
    return tt->m_coverImage;
}

const QMultiMap<MetadataFields::ColumnsRoles, QVariant> &TrackTags::roleMapping() const {
    return tt->m_roleMapping;
}

QVariant TrackTags::value(const MetadataFields::ColumnsRoles role) const {
    return tt->m_roleMapping.value(role);
}

void TrackTags::add(const MetadataFields::ColumnsRoles role, const QVariant &value) const {
    if (tt->m_roleMapping.values(role).contains(value)) return;

    tt->m_roleMapping.insert(role, value);
}

void TrackTags::addCoverImage(const QByteArray &image) const {
    if (image.isEmpty()) {
        add(MetadataFields::HasEmbeddedCover, false);
    } else {
        add(MetadataFields::HasEmbeddedCover, true);
        tt->m_coverImage = image;
    }
}
