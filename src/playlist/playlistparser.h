#ifndef PLAYLISTPARSER_H
#define PLAYLISTPARSER_H

#include <QUrl>
#include <QList>
#include <QString>

class M3UPlaylistParser {
public:
    QList<QUrl> fromPlaylist(const QUrl &m3uFile, const QByteArray &content);
    QString toPlaylist(const QUrl &m3uFile, const QList<QString> &urls);
};

class PlaylistParser {
public:
    QList<QUrl> fromPlaylist(const QUrl &m3uFile, const QByteArray &content);
    QString toPlaylist(const QUrl &m3uFile, const QList<QString> &urls);
};

#endif //PLAYLISTPARSER_H
