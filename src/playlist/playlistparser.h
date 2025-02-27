#ifndef PLAYLISTPARSER_H
#define PLAYLISTPARSER_H

#include <QList>
#include <QString>
#include <QUrl>

class M3UPlaylistParser {
public:
    static QList<QUrl> fromPlaylist(const QUrl& m3uFile, const QByteArray& content);
    static QString toPlaylist(const QUrl& m3uFile, const QList<QString>& urls);
};

class PlaylistParser {
public:
    static QList<QUrl> fromPlaylist(const QUrl& m3uFile, const QByteArray& content);
    static QString toPlaylist(const QUrl& m3uFile, const QList<QString>& urls);
};

#endif // PLAYLISTPARSER_H
