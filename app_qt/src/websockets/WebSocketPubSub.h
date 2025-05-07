#ifndef WEBSOCKETPUBSUB_H
#define WEBSOCKETPUBSUB_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QString>
#include <QSet>
#include <QTimer>

class WebSocketPubSub : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketPubSub(QObject *parent = nullptr);
    ~WebSocketPubSub();

    Q_INVOKABLE void connectToUrl(const QUrl &url, bool reconnect = true);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void subscribe(const QString &topic, bool force = false);
    Q_INVOKABLE void unsubscribe(const QString &topic);
    Q_INVOKABLE void publish(const QString &topic, const QString &payload);

    // Getter for connection state
    Q_INVOKABLE bool isConnected() const;

public slots:
    void onMessageReceived(const QString &message);
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onPing(const QByteArray &payload);
    void onPong(const QByteArray &payload);

signals:
    void messageReceived(const QString &action, const QString &topic, const QString &payload);
    void connectionStateChanged(bool connected);

private:
    void scheduleReconnect();
    void startHeartbeatMonitor();
    void stopHeartbeatMonitor();
    void checkHeartbeat();
    void sendPing();

    // Add these new members
    QTimer m_pingTimeoutTimer;
    const int PING_TIMEOUT = 5000; // 5 seconds to wait for pong response
    qint64 m_lastPingId;
    QMap<qint64, QDateTime> m_pendingPings;
    QByteArray m_lastPingPayload;

    // New helper methods
    void resetPingTimeout();
    void handlePingTimeout();

    QWebSocket m_webSocket;
    QSet<QString> m_subscribedTopics; // Store subscribed topics

    // reconnection logic
    bool reconnects = true;
    bool m_reconnectScheduled = false;
    QUrl m_lastUrl;
    int m_reconnectAttempts = 0;

    // heartbeat monitoring
    QTimer m_heartbeatTimer;
    QTimer m_pingTimer;
    int m_missedHeartbeats = 0;
    const int MAX_MISSED_HEARTBEATS = 3;
    const int HEARTBEAT_INTERVAL = 15000; // 15 seconds
    const int PING_INTERVAL = 30000;      // 30 seconds
    QDateTime m_lastMessageTime;
};

#endif // WEBSOCKETPUBSUB_H