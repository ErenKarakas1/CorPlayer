#include "corplayer.h"

#include <QDir>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QQmlContext>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CorPlayer"));
    QApplication::setOrganizationName(QStringLiteral("Corp"));
    QQmlApplicationEngine engine;

    CorPlayer corPlayer;
    const auto qmlUrl = QStringLiteral("CorPlayer");

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
