#ifndef WEBSOCKETPUBSUB_H
#define WEBSOCKETPUBSUB_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QString>
#include <QSet>

class WebSocketPubSub : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketPubSub(QObject *parent = nullptr);
    ~WebSocketPubSub();

    Q_INVOKABLE void connectToUrl(const QUrl &url);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void subscribe(const QString &topic);
    Q_INVOKABLE void unsubscribe(const QString &topic);
    Q_INVOKABLE void publish(const QString &topic, const QString &message);
    
public slots:
    void onMessageReceived(const QString &message);
    void onConnected();
    void onError(QAbstractSocket::SocketError error);

signals:
    void messageReceived(const QString &topic, const QString &message);

private:
    QWebSocket m_webSocket;
    QSet<QString> m_subscribedTopics; // Store subscribed topics
};

#endif // WEBSOCKETPUBSUB_H
