#include "metadata.hpp"

#include "playerutils.hpp"

bool Metadata::EntryFields::isValid() const {
    return !trackFields.isEmpty() || !title.isEmpty() || !url.isEmpty();
}

void Metadata::TrackFields::insert(const Fields field, const QVariant& value) {
    data.insert(field, value);
}

bool Metadata::TrackFields::isValid() const {
    return !isEmpty() && data.value(Duration).toTime().isValid();
}

bool Metadata::TrackFields::isEmpty() const {
    return data.isEmpty();
}

QVariant Metadata::TrackFields::get(const Fields field) const {
    return data.value(field);
}

bool Metadata::TrackFields::contains(const Fields field) const {
    return data.contains(field);
}

QString Metadata::TrackFields::generateHash() const {
    return PlayerUtils::calculateTrackHash(data.value(Title).toString(), data.value(Artist).toString(),
                                           data.value(Album).toString(), data.value(AlbumArtist).toString(),
                                           data.value(Genre).toString(), data.value(Year).toString(),
                                           data.value(Duration).toString(), data.value(BitRate).toString(),
                                           data.value(ResourceUrl).toUrl().toString(), data.value(FileType).toString());
}
