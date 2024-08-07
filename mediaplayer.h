#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <qobject.h>

class MediaPlayer : public QObject
{
public:
    explicit MediaPlayer(QObject *parent = nullptr);

signals:
};

#endif // MEDIAPLAYER_H
