#include "WebSocketPubSub.h"
#include "WebSocketAction.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtCore/qmath.h>  // For qPow
#include <QtCore/QtGlobal> // For qMin

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

void WebSocketPubSub::connectToUrl(const QUrl &url, bool reconnects)
{
    this->reconnects = reconnects;
    this->m_lastUrl = url;

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // Disable verification (⚠️ development only)

    this->m_webSocket.setSslConfiguration(sslConfig);

    // Connect to SSL error signal
    connect(&this->m_webSocket, &QWebSocket::sslErrors,
            this, [](const QList<QSslError> &errors)
            {
                    bool selfSigned = false;
                    for (const QSslError &error : errors) {
                        if (error.error() == QSslError::SelfSignedCertificate ||
                            error.error() == QSslError::SelfSignedCertificateInChain) {
                            selfSigned = true;
                        }
                    }
                    if (selfSigned) {
                        qWarning() << "🔒 SELF-SIGNED CERT, DEV ONLY";
                    } else {
                        qInfo() << "✅ PRODUCTION READY, CERTIFICATE APPROVED!";
                    } });

    this->m_webSocket.open(url);
    qDebug() << "OPEN WebSockets " << url.toString();
}

void WebSocketPubSub::disconnect()
{
    this->m_webSocket.close();
    qDebug() << "CLOSE WebSockets";
}

void WebSocketPubSub::subscribe(const QString &topic, bool force)
{

    if (force || !this->m_subscribedTopics.contains(topic))
    {
        this->m_subscribedTopics.insert(topic); // Keep track of the subscribed topics

        if (this->m_webSocket.state() == QAbstractSocket::ConnectedState)
        {
            // Send a subscription message to the WebSocket
            QJsonObject jsonMessage;
            jsonMessage["action"] = actionToString(WebSocketAction::SUBSCRIBE);
            jsonMessage["topic"] = topic;
            this->m_webSocket.sendTextMessage(QJsonDocument(jsonMessage).toJson());
        }
    }
}

void WebSocketPubSub::unsubscribe(const QString &topic)
{
    if (this->m_subscribedTopics.contains(topic))
    {
        this->m_subscribedTopics.remove(topic);

        if (this->m_webSocket.state() == QAbstractSocket::ConnectedState)
        {
            // Send an unsubscribe message to the WebSocket
            QJsonObject jsonMessage;
            jsonMessage["action"] = actionToString(WebSocketAction::UNSUBSCRIBE);
            jsonMessage["topic"] = topic;
            this->m_webSocket.sendTextMessage(QJsonDocument(jsonMessage).toJson());
        }
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
    qDebug() << "WebSocket connected";
    this->m_reconnectAttempts = 0;

    qDebug() << "Subscribed topics (length - " << this->m_subscribedTopics.count() << " ):";
    // Slight delay before subscribing, to let the connection settle
    for (const QString &topic : this->m_subscribedTopics)
    {
        qDebug() << topic;
        qDebug() << "WebSocket state before subscribe:" << this->m_webSocket.state();
        this->subscribe(topic, /*force=*/true);
    }
}

void WebSocketPubSub::onMessageReceived(const QString &msg)
{
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    if (!doc.isObject())
        return;

    auto obj = doc.object();
    const QString action = obj["action"].toString();
    const QString topic = obj["topic"].toString();
    const QString payload = obj["payload"].toString();

    emit this->messageReceived(action, topic, payload);
}

void WebSocketPubSub::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << this->m_webSocket.errorString() << " (" << error << ")";

    if (error == QAbstractSocket::RemoteHostClosedError ||
        error == QAbstractSocket::ConnectionRefusedError ||
        error == QAbstractSocket::NetworkError)
    {
        this->m_webSocket.abort(); // Abort current socket
        this->scheduleReconnect();
    }
}

// reconnection logic
void WebSocketPubSub::scheduleReconnect()
{
    // Don't schedule multiple reconnects or try to reconnect when already connecting
    if (this->m_reconnectScheduled || this->m_webSocket.state() == QAbstractSocket::ConnectingState)
        return;

    this->m_reconnectScheduled = true;

    // Cap the reconnect attempts to prevent integer overflow
    const int maxAttempts = 10;
    if (this->m_reconnectAttempts >= maxAttempts)
    {
        this->m_reconnectAttempts = maxAttempts;
    }

    // Base delay is 1000ms (1 second)
    // Use a proper exponential backoff with jitter for better network behavior
    int baseDelay = 1000;
    int maxDelay = 30000; // 30 seconds max delay

    // Calculate delay with exponential backoff (1.5^n rather than 2^n for smoother growth)
    double factor = std::pow(1.5, this->m_reconnectAttempts);
    int delay = static_cast<int>(baseDelay * factor);

    // Add jitter (±20% randomness) to prevent reconnection storms
    int jitter = static_cast<int>(delay * 0.2 * (static_cast<double>(rand()) / RAND_MAX - 0.5));
    delay += jitter;

    // Ensure delay is within bounds (minimum 1s, maximum 30s)
    delay = qBound(1000, delay, maxDelay);

    qDebug() << "Scheduling reconnect #" << (this->m_reconnectAttempts + 1)
             << "in" << delay << "ms";

    QTimer::singleShot(delay, this, [this]()
                       {
        this->m_reconnectScheduled = false;

        if (this->m_webSocket.state() == QAbstractSocket::UnconnectedState) {
            qDebug() << "Attempting reconnect #" << (this->m_reconnectAttempts + 1);
            connectToUrl(this->m_lastUrl);
            this->m_reconnectAttempts++;
        } else {
            qDebug() << "Reconnect skipped, state:" << this->m_webSocket.state();
        } });
}
