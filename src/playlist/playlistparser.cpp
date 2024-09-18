#include <playlist/playlistparser.h>

#include <QMimeDatabase>

QList<QUrl> M3UPlaylistParser::fromPlaylist(const QUrl &m3uFile, const QByteArray &content) {
    Q_UNUSED(m3uFile);
    QList<QUrl> result;

    const auto lines = content.split('\n');
    for (const QByteArray &l : lines) {
        const QString &line = QString::fromUtf8(l);
        if (!line.isEmpty() && !line.startsWith(QStringLiteral("#"))) {
            const QUrl &url = line.contains(QStringLiteral("://")) ? QUrl(line) : QUrl::fromLocalFile(line);
            result.append(url);
        }
    }
    return result;
}

QString M3UPlaylistParser::toPlaylist(const QUrl &m3uFile, const QList<QString> &urls) {
    Q_UNUSED(m3uFile);
    QString result;

    for (const QString &line : urls) {
        if (!line.isEmpty()) {
            result += line + QStringLiteral("\n");
        }
    }
    return result;
}

QList<QUrl> PlaylistParser::fromPlaylist(const QUrl &m3uFile, const QByteArray &content) {
    QList<QUrl> result;

    if (m3uFile.isValid() && !m3uFile.isEmpty()) {
        auto mimeType = QMimeDatabase().mimeTypeForUrl(m3uFile);

        if (mimeType.name().contains(QStringLiteral("mpegurl"))) {
            result = M3UPlaylistParser::fromPlaylist(m3uFile, content);
        }
    }

    return result;
}

QString PlaylistParser::toPlaylist(const QUrl &m3uFile, const QList<QString> &urls) {
    QString result;

    if (m3uFile.isValid() && !m3uFile.isEmpty()) {
        auto mimeType = QMimeDatabase().mimeTypeForUrl(m3uFile);

        if (mimeType.name().contains(QStringLiteral("mpegurl"))) {
            result = M3UPlaylistParser::toPlaylist(m3uFile, urls);
        }
    }

    return result;
}
