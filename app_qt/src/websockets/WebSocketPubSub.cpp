#include "WebSocketPubSub.h"
// #include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>

WebSocketPubSub::WebSocketPubSub(QObject *parent)
    : QObject(parent)
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
    qDebug() << "OPEN";
}

void WebSocketPubSub::disconnect()
{
    this->m_webSocket.close();
    qDebug() << "CLOSE";
}

void WebSocketPubSub::subscribe(const QString &topic)
{
    if (this->m_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        if (!this->m_subscribedTopics.contains(topic))
        {
            // Send a subscription message to the WebSocket
            QJsonObject jsonMessage;
            jsonMessage["action"] = "subscribe";
            jsonMessage["topic"] = topic;
            auto json = QJsonDocument(jsonMessage).toJson();
            this->m_webSocket.sendTextMessage(json);

            this->m_subscribedTopics.insert(topic); // Keep track of the subscribed topics
            qDebug() << "Subscribed: " << json;
        }
    }
}

void WebSocketPubSub::unsubscribe(const QString &topic)
{
    if (this->m_webSocket.state() == QAbstractSocket::ConnectedState && this->m_subscribedTopics.contains(topic))
    {
        // Send an unsubscribe message to the WebSocket
        QJsonObject jsonMessage;
        jsonMessage["action"] = "unsubscribe";
        jsonMessage["topic"] = topic;
        auto json = QJsonDocument(jsonMessage).toJson();
        this->m_webSocket.sendTextMessage(json);

        this->m_subscribedTopics.remove(topic); // Remove from the subscribed topics list
        qDebug() << "Unsubscribed: " << json;
    }
}

void WebSocketPubSub::publish(const QString &topic, const QString &message)
{
    if (this->m_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        // Send a publish message to the WebSocket
        QJsonObject jsonMessage;
        jsonMessage["action"] = "publish";
        jsonMessage["topic"] = topic;
        jsonMessage["message"] = message;
        auto json = QJsonDocument(jsonMessage).toJson();
        this->m_webSocket.sendTextMessage(json);
        qDebug() << "Published: " << json;
    }
}

void WebSocketPubSub::onConnected()
{
    // Optionally handle server-side auto-subscribe or status messages
}

void WebSocketPubSub::onMessageReceived(const QString &msg)
{
    qDebug() << "INIT Received: " << msg;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    if (!doc.isObject())
        return;

    auto obj = doc.object();
    const QString topic = obj["topic"].toString();
    const QString message = obj["message"].toString();

    emit messageReceived(topic, message);

    qDebug() << "Received: " << msg;
}

void WebSocketPubSub::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << m_webSocket.errorString() << " (" << error << ")";
}