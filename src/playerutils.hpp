#ifndef PLAYERUTILS_H
#define PLAYERUTILS_H

#include <QCryptographicHash>
#include <QMimeType>
#include <QQmlEngine>

#include <concepts>

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

bool isPlaylist(const QMimeType& mimeType);

template <typename T>
concept HasToUtf8 = requires(T t) {
    { t.toUtf8() } -> std::convertible_to<QByteArray>;
};

template <typename... Columns>
QString calculateTrackHash(const Columns&... columns) {
    static_assert((HasToUtf8<Columns> && ...), "All columns must have a toUtf8() method returning QByteArray");

    QCryptographicHash hash{QCryptographicHash::Algorithm::Md5};
    (hash.addData(columns.toUtf8()), ...);
    return QString::fromUtf8(hash.result().toHex());
}

} // namespace PlayerUtils

#endif // PLAYERUTILS_H
