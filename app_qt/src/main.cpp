#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "websockets/WebSocketPubSub.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Register WebSocketPubSub class with QML
    qmlRegisterType<WebSocketPubSub>("MyApp", 1, 0, "WebSocketPubSub");

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));

    QQmlApplicationEngine engine;
    engine.load(url);

    return app.exec();
}
