#ifndef PLAYERUTILS_H
#define PLAYERUTILS_H

#include <QMimeType>
#include <QQmlEngine>

namespace PlayerUtils {
Q_NAMESPACE
QML_NAMED_ELEMENT(PlayerUtils)

enum PlaylistEnqueueMode {
    AppendPlaylist,
    ReplacePlaylist,
    AfterCurrentTrack,
};

Q_ENUM_NS(PlaylistEnqueueMode)

enum PlaylistEnqueueTriggerPlay {
    DoNotTriggerPlay,
    TriggerPlay,
};

Q_ENUM_NS(PlaylistEnqueueTriggerPlay)

enum SkipReason {
    Automatic, // song ended, etc.
    Manual,    // user pressed skip forward button, etc.
};

Q_ENUM_NS(SkipReason)

enum PlaylistEntryType {
    Album,
    Artist,
    Genre,
    Lyricist,
    Composer,
    Track,
    FileName,
    Container,
    Playlist,
    Unknown,
};

Q_ENUM_NS(PlaylistEntryType)

enum FilterType {
    UnknownFilter,
    NoFilter,
    FilterById,
    FilterByGenre,
    FilterByArtist,
    FilterByGenreAndArtist,
    FilterByRecentlyPlayed,
    FilterByFrequentlyPlayed,
    FilterByPath,
};

Q_ENUM_NS(FilterType)

bool isPlaylist(const QMimeType &mimeType);
} // namespace PlayerUtils

#endif // PLAYERUTILS_H
