#include "corplayer.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CorPlayer"));
    QApplication::setOrganizationName(QStringLiteral("Corp"));
    QQmlApplicationEngine engine;

    CorPlayer corPlayer;
    const auto qmlUrl = QStringLiteral("CorPlayer");

    engine.rootContext()->setContextProperty(qmlUrl, &corPlayer);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.loadFromModule(qmlUrl, "Main");

    return QApplication::exec();
}
