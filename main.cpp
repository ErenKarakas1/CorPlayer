#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <qqmlcontext.h>
#include "MediaPlayer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    MediaPlayer mediaPlayerBackend;

    engine.rootContext()->setContextProperty("mediaPlayerBackend", &mediaPlayerBackend);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("mplayer", "Main");

    return app.exec();
}
