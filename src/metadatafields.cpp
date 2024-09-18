#include "metadatafields.h"

bool MetadataFields::TrackMetadataField::albumInfoIsSame(const TrackMetadataField &other) const {
    return hasRole(AlbumRole) == other.hasRole(AlbumRole) && album() == other.album() &&
           hasRole(AlbumArtistRole) == other.hasRole(AlbumArtistRole) && albumArtist() == other.albumArtist();
}

bool MetadataFields::TrackMetadataField::isSameTrack(const TrackMetadataField &other) const {
    return title() == other.title() && hasRole(AlbumRole) == other.hasRole(AlbumRole) &&
           (hasRole(AlbumRole) ? album() == other.album() : true) && hasRole(ArtistRole) == other.hasRole(ArtistRole) &&
           (hasRole(ArtistRole) ? artist() == other.artist() : true) &&
           hasRole(AlbumArtistRole) == other.hasRole(AlbumArtistRole) &&
           (hasRole(AlbumArtistRole) ? albumArtist() == other.albumArtist() : true) &&
           hasRole(TrackNumberRole) == other.hasRole(TrackNumberRole) &&
           (hasRole(TrackNumberRole) ? trackNumber() == other.trackNumber() : true) &&
           hasRole(DiscNumberRole) == other.hasRole(DiscNumberRole) &&
           (hasRole(DiscNumberRole) ? discNumber() == other.discNumber() : true) && duration() == other.duration() &&
           resourceURI() == other.resourceURI() && hasRole(GenreRole) == other.hasRole(GenreRole) &&
           (hasRole(GenreRole) ? genre() == other.genre() : true) &&
           hasRole(ComposerRole) == other.hasRole(ComposerRole) &&
           (hasRole(ComposerRole) ? composer() == other.composer() : true) &&
           hasRole(LyricistRole) == other.hasRole(LyricistRole) &&
           (hasRole(LyricistRole) ? lyricist() == other.lyricist() : true) &&
           hasRole(CommentRole) == other.hasRole(CommentRole) &&
           (hasRole(CommentRole) ? comment() == other.comment() : true) &&
           hasRole(YearRole) == other.hasRole(YearRole) && (hasRole(YearRole) ? year() == other.year() : true) &&
           hasRole(ChannelsRole) == other.hasRole(ChannelsRole) &&
           (hasRole(ChannelsRole) ? channels() == other.channels() : true) &&
           hasRole(BitRateRole) == other.hasRole(BitRateRole) &&
           (hasRole(BitRateRole) ? bitRate() == other.bitRate() : true) &&
           hasRole(SampleRateRole) == other.hasRole(SampleRateRole) &&
           (hasRole(SampleRateRole) ? sampleRate() == other.sampleRate() : true);
}
