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

    Q_INVOKABLE void connectToUrl(const QUrl &url, bool reconnect = false);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void subscribe(const QString &topic, bool force = false);
    Q_INVOKABLE void unsubscribe(const QString &topic);
    Q_INVOKABLE void publish(const QString &topic, const QString &payload);

public slots:
    void onMessageReceived(const QString &message);
    void onConnected();
    void onError(QAbstractSocket::SocketError error);

signals:
    void messageReceived(const QString &action, const QString &topic, const QString &payload);

private:
    void scheduleReconnect();

    QWebSocket m_webSocket;
    QSet<QString> m_subscribedTopics; // Store subscribed topics

    // reconnection logic
    bool reconnects = true;

    bool m_reconnectScheduled = false;
    QTimer m_reconnectTimer;
    QUrl m_lastUrl;
    int m_reconnectAttempts = 0;
};

#endif // WEBSOCKETPUBSUB_H
