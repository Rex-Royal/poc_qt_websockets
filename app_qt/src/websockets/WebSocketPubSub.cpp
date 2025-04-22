#include "WebSocketPubSub.h"
#include <QJsonDocument>
#include <QJsonObject>

enum WebSocketAction
{
    SUBSCRIBE = 0,
    PUBLISH = 1,
    UNSUBSCRIBE = 2
};

QString actionToString(WebSocketAction action)
{
    switch (action)
    {
    case SUBSCRIBE:
        return "subscribe";
    case PUBLISH:
        return "publish";
    case UNSUBSCRIBE:
        return "unsubscribe";
    default:
        return "unknown";
    }
}

WebSocketPubSub::WebSocketPubSub(QObject *parent) : QObject(parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketPubSub::onConnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketPubSub::onMessageReceived);
    connect(&m_webSocket, static_cast<void (QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            this, &WebSocketPubSub::onError);
}

WebSocketPubSub::~WebSocketPubSub()
{
    this->disconnect();
}

void WebSocketPubSub::connectToUrl(const QUrl &url)
{
    this->m_webSocket.open(url);
    qDebug() << "OPEN WebSockets " << url.toString();
}

void WebSocketPubSub::disconnect()
{
    this->m_webSocket.close();
    qDebug() << "CLOSE WebSockets";
}

void WebSocketPubSub::subscribe(const QString &topic)
{
    if (this->m_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        if (!this->m_subscribedTopics.contains(topic))
        {
            // Send a subscription message to the WebSocket
            QJsonObject jsonMessage;
            jsonMessage["action"] = actionToString(WebSocketAction::SUBSCRIBE);
            jsonMessage["topic"] = topic;
            this->m_webSocket.sendTextMessage(QJsonDocument(jsonMessage).toJson());

            this->m_subscribedTopics.insert(topic); // Keep track of the subscribed topics
        }
    }
}

void WebSocketPubSub::unsubscribe(const QString &topic)
{
    if (this->m_webSocket.state() == QAbstractSocket::ConnectedState && this->m_subscribedTopics.contains(topic))
    {
        // Send an unsubscribe message to the WebSocket
        QJsonObject jsonMessage;
        jsonMessage["action"] = actionToString(WebSocketAction::UNSUBSCRIBE);
        jsonMessage["topic"] = topic;
        this->m_webSocket.sendTextMessage(QJsonDocument(jsonMessage).toJson());

        this->m_subscribedTopics.remove(topic);
    }
}

void WebSocketPubSub::publish(const QString &topic, const QString &payload)
{
    if (this->m_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        // Send a publish message to the WebSocket
        QJsonObject jsonMessage;
        jsonMessage["action"] = actionToString(WebSocketAction::PUBLISH);
        jsonMessage["topic"] = topic;
        jsonMessage["payload"] = payload;
        this->m_webSocket.sendTextMessage(QJsonDocument(jsonMessage).toJson());
    }
}

void WebSocketPubSub::onConnected()
{
    // Optionally handle server-side auto-subscribe or status messages
}

void WebSocketPubSub::onMessageReceived(const QString &msg)
{
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    if (!doc.isObject())
        return;

    auto obj = doc.object();
    const QString topic = obj["topic"].toString();
    const QString payload = obj["payload"].toString();

    emit messageReceived(topic, payload);
}

void WebSocketPubSub::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << m_webSocket.errorString() << " (" << error << ")";
}