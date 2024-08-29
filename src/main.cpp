#include "corplayer.h"

#include <QDir>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QQmlContext>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    CorPlayer corPlayer;
    constexpr auto qmlUrl = "CorPlayer";

    engine.rootContext()->setContextProperty(qmlUrl, &corPlayer);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    if (QFontDatabase::addApplicationFont(":/fonts/FontAwesome.otf") < 0) {
        qWarning() << "Failed to load FontAwesome.otf";
    }

    engine.loadFromModule(qmlUrl, "Main");

    return app.exec();
}
