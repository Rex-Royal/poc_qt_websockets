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
    QCoreApplication::setApplicationVersion("1.0.1");

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

    QCommandLineOption secureOption(QStringList() << "s" << "secure",
                                    "Use secure WebSocket (WSS)");
    parser.addOption(secureOption);

    QCommandLineOption certOption(QStringList() << "c" << "cert",
                                  "Path to SSL certificate file (PEM format)",
                                  "certificate");
    parser.addOption(certOption);

    QCommandLineOption keyOption(QStringList() << "k" << "key",
                                 "Path to SSL private key file (PEM format)",
                                 "key");
    parser.addOption(keyOption);

    parser.process(app);

    int port = parser.value(portOption).toInt();
    bool secure = parser.isSet(secureOption);

    // Get broker instance
    WebSocketBroker *broker = WebSocketBroker::getInstance(port, secure);

    // Configure SSL if needed
    if (secure)
    {
        if (!parser.isSet(certOption) || !parser.isSet(keyOption))
        {
            qCritical() << "Secure mode requires both certificate and key files!";
            return 1;
        }

        QString certPath = parser.value(certOption);
        QString keyPath = parser.value(keyOption);

        if (!broker->configureSsl(certPath, keyPath))
        {
            qCritical() << "Failed to configure SSL. Please check certificate and key files.";
            return 1;
        }
    }

    // Check if broker started successfully
    if (!broker->isRunning())
    {
        qWarning().noquote() << "Failed to start WebSocket Broker, but recovery will be attempted.";
    }

    // Connect application quit signal to stop the broker
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [broker]() {
        if (broker)
            broker->stop();
    });

    return app.exec();
}