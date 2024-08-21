#include "corplayer.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QQmlContext>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QFontDatabase::addApplicationFont(":/FontAwesome.otf");

    CorPlayer corPlayer;
    constexpr auto qmlUrl = "CorPlayer";

    engine.rootContext()->setContextProperty(qmlUrl, &corPlayer);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule(qmlUrl, "Main");

    return app.exec();
}
