#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QThread>
#include <csignal>
#include "./websockets/WebSocketBroker.h"

// Signal handler to capture SIGINT (Ctrl+C)
void signalHandler(int signal)
{
    qDebug().noquote() << "Signal" << signal << "received, shutting down...";
    QCoreApplication::quit();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("WebSocketBroker");
    QCoreApplication::setApplicationVersion("1.0.0");

    // Setup signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("WebSocket Broker for pub/sub messaging");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port",
                                  "Port to listen on (default: 3001)",
                                  "port", "3001");
    parser.addOption(portOption);

    parser.process(app);

    int port = parser.value(portOption).toInt();

    // Get broker instance
    WebSocketBroker *broker = WebSocketBroker::getInstance(port);

    // Check if broker started successfully
    if (!broker->isRunning())
    {
        qWarning().noquote() << "Failed to start WebSocket Broker, but recovery will be attempted.";
    }

    // Connect application quit signal to stop the broker
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [broker]()
                     {
        qDebug().noquote() << "Application quitting, stopping broker...";
        broker->stop(); });

    return app.exec();
}