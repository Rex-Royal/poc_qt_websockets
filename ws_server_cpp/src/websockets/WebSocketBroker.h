#ifndef WEBSOCKETBROKER_H
#define WEBSOCKETBROKER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QMap>
#include <QSet>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>

enum class WebSocketAction
{
    SUBSCRIBE,
    UNSUBSCRIBE,
    PUBLISH,
    BROADCAST
};

class WebSocketBroker : public QObject
{
    Q_OBJECT

public:
    static WebSocketBroker *getInstance(int port = 3001, bool secure = false);
    virtual ~WebSocketBroker();

    // Publish a message to all clients subscribed to a topic
    void publish(const QString &topic, const QString &payload, QWebSocket *excludeClient = nullptr);

    // Broadcast a message to all connected clients
    void broadcast(const QString &payload, QWebSocket *excludeClient = nullptr);

    // Check if server is running
    bool isRunning() const;

    // Explicitly start/stop the server
    bool start();
    bool stop();

    // Get current server port
    int serverPort() const;

    // SSL configuration
    bool configureSsl(const QString &certPath, const QString &keyPath);

private:
    explicit WebSocketBroker(int port = 3001, bool secure = false, QObject *parent = nullptr);
    static WebSocketBroker *instance;

    struct ClientInfo
    {
        QSet<QString> topics;
        QString clientId;
        QHostAddress address;
        quint16 port;
    };

    QWebSocketServer *m_server;
    QMap<QWebSocket *, ClientInfo> m_clients;
    QTimer *m_healthCheckTimer;
    QTimer *m_recoveryTimer;
    int m_port;
    int m_recoveryAttempts;
    bool m_isRecovering;
    bool m_secure;
    QSslConfiguration m_sslConfiguration;

    // Helper methods
    QString actionToString(WebSocketAction action);
    WebSocketAction stringToAction(const QString &action);
    QString generateClientId();
    void handleMessage(const QString &message, QWebSocket *client);
    void scheduleServerRecovery();

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onClientDisconnected();
    void onServerError(QWebSocketProtocol::CloseCode closeCode);
    void onSslErrors(const QList<QSslError> &errors);
    void performHealthCheck();
    void attemptServerRecovery();
};

#endif // WEBSOCKETBROKER_H