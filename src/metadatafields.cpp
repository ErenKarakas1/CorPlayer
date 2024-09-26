#include "metadatafields.h"

bool MetadataFields::TrackMetadataField::albumInfoIsSame(const TrackMetadataField &other) const {
    return hasRole(AlbumRole) == other.hasRole(AlbumRole) && album() == other.album() &&
           hasRole(AlbumArtistRole) == other.hasRole(AlbumArtistRole) && albumArtist() == other.albumArtist();
}

bool MetadataFields::TrackMetadataField::isSameTrack(const TrackMetadataField &other) const {
    return hash() == other.hash();
}
