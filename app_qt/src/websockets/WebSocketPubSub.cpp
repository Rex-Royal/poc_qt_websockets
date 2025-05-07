#include "WebSocketPubSub.h"
#include "WebSocketAction.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtCore/qmath.h>
#include <QtCore/QtGlobal>
#include <QDateTime>
#include <QRandomGenerator>

const char *socketStateToString(QAbstractSocket::SocketState state)
{
    switch (state)
    {
    case QAbstractSocket::UnconnectedState:
        return "Unconnected";
    case QAbstractSocket::HostLookupState:
        return "HostLookup";
    case QAbstractSocket::ConnectingState:
        return "Connecting";
    case QAbstractSocket::ConnectedState:
        return "Connected";
    case QAbstractSocket::BoundState:
        return "Bound";
    case QAbstractSocket::ClosingState:
        return "Closing";
    case QAbstractSocket::ListeningState:
        return "Listening";
    default:
        return "Unknown";
    }
}

WebSocketPubSub::WebSocketPubSub(QObject *parent) : QObject(parent)
{
    // Existing connections
    connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketPubSub::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketPubSub::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketPubSub::onMessageReceived);
    connect(&m_webSocket, static_cast<void (QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            this, &WebSocketPubSub::onError);

    connect(&m_webSocket, &QWebSocket::stateChanged, this, [this](QAbstractSocket::SocketState state)
            { qDebug() << "stateChanged: " << socketStateToString(state); });

    // No need to connect to pingReceived - QWebSocket doesn't have this signal
    // We'll use the existing onPing slot for manually receiving ping messages

    // Setup heartbeat timer
    m_heartbeatTimer.setInterval(HEARTBEAT_INTERVAL);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &WebSocketPubSub::checkHeartbeat);

    // Setup ping timer
    m_pingTimer.setInterval(PING_INTERVAL);
    connect(&m_pingTimer, &QTimer::timeout, this, &WebSocketPubSub::sendPing);

    // Setup ping timeout timer
    m_pingTimeoutTimer.setInterval(PING_TIMEOUT);
    m_pingTimeoutTimer.setSingleShot(true);
    connect(&m_pingTimeoutTimer, &QTimer::timeout, this, &WebSocketPubSub::handlePingTimeout);

    // Initialize ping ID
    m_lastPingId = 0;
}

void WebSocketPubSub::resetPingTimeout()
{
    // Reset the ping timeout timer
    if (m_pingTimeoutTimer.isActive())
    {
        m_pingTimeoutTimer.stop();
    }
    m_pingTimeoutTimer.start();
}

void WebSocketPubSub::handlePingTimeout()
{
    // Check if we're already disconnected or disconnecting
    if (m_webSocket.state() == QAbstractSocket::UnconnectedState ||
        m_webSocket.state() == QAbstractSocket::ClosingState)
    {
        return;
    }

    qWarning() << "Ping timeout occurred! No pong response received within" << PING_TIMEOUT << "ms";

    // Check if we have any pending pings older than PING_TIMEOUT
    QDateTime currentTime = QDateTime::currentDateTime();
    bool hasTimedOutPings = false;

    QMutableMapIterator<qint64, QDateTime> i(m_pendingPings);
    while (i.hasNext())
    {
        i.next();
        if (i.value().msecsTo(currentTime) > PING_TIMEOUT)
        {
            qWarning() << "Ping ID" << i.key() << "timed out after"
                       << i.value().msecsTo(currentTime) << "ms";
            i.remove();
            hasTimedOutPings = true;
        }
    }

    if (hasTimedOutPings)
    {
        // Increment missed heartbeats
        m_missedHeartbeats++;

        // If we've missed too many heartbeats, consider the connection dead
        if (m_missedHeartbeats >= MAX_MISSED_HEARTBEATS)
        {
            qWarning() << "Connection appears dead after" << m_missedHeartbeats
                       << "missed responses";

            // Close and abort the socket to force a reconnection
            m_webSocket.abort();

            // Emit connection state changed
            emit connectionStateChanged(false);

            // Try to reconnect if enabled
            if (reconnects)
            {
                scheduleReconnect();
            }
        }
    }
}

WebSocketPubSub::~WebSocketPubSub()
{
    this->disconnect();
}

void WebSocketPubSub::connectToUrl(const QUrl &url, bool reconnects)
{
    // Cancel any existing reconnect timers
    if (m_reconnectScheduled)
    {
        // Stop any pending reconnect timer
        // This requires adding a QTimer m_reconnectTimer member
        // m_reconnectTimer.stop();
        m_reconnectScheduled = false;
    }

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

    // Attempt connection
    this->m_webSocket.open(url);
    qDebug() << "OPEN WebSockets " << url.toString();
}

void WebSocketPubSub::disconnect()
{
    stopHeartbeatMonitor();

    // Stop all timers first
    m_heartbeatTimer.stop();
    m_pingTimer.stop();
    m_pingTimeoutTimer.stop();

    // Disconnect all signals before closing the socket
    m_webSocket.disconnect();

    // Then close the socket
    m_webSocket.close();

    qDebug() << "CLOSE WebSockets";
}

bool WebSocketPubSub::isConnected() const
{
    return m_webSocket.state() == QAbstractSocket::ConnectedState;
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

    // Start heartbeat monitoring
    startHeartbeatMonitor();

    // Update last message time
    m_lastMessageTime = QDateTime::currentDateTime();

    // Emit connection state changed
    emit connectionStateChanged(true);

    qDebug() << "Resubscribing to topics (count: " << this->m_subscribedTopics.count() << ")";
    // Resubscribe to all topics
    for (const QString &topic : this->m_subscribedTopics)
    {
        qDebug() << "Resubscribing to:" << topic;
        qDebug() << "WebSocket state before subscribe:" << this->m_webSocket.state();
        this->subscribe(topic, /*force=*/true);
    }

    // // Also subscribe to system topics for more robust monitoring
    // this->subscribe("system", true);
}

void WebSocketPubSub::onDisconnected()
{
    qDebug() << "WebSocket disconnected";

    // Stop timers
    stopHeartbeatMonitor();
    if (m_pingTimeoutTimer.isActive())
    {
        m_pingTimeoutTimer.stop();
    }

    // Clear pending pings
    m_pendingPings.clear();

    // Reset counters
    m_missedHeartbeats = 0;

    // Emit connection state changed
    emit connectionStateChanged(false);

    // Try to reconnect if enabled and not already scheduled
    if (reconnects && !m_reconnectScheduled)
    {
        scheduleReconnect();
    }
}

void WebSocketPubSub::onMessageReceived(const QString &msg)
{
    // Update last message time
    m_lastMessageTime = QDateTime::currentDateTime();

    // Reset missed heartbeats counter since we're receiving messages
    m_missedHeartbeats = 0;

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

    // Stop timers first to prevent callbacks during reconnection
    stopHeartbeatMonitor();
    if (m_pingTimeoutTimer.isActive())
    {
        m_pingTimeoutTimer.stop();
    }

    // Only proceed with reconnection if we're not already in the process
    if (m_webSocket.state() != QAbstractSocket::ConnectingState &&
        m_webSocket.state() != QAbstractSocket::HostLookupState)
    {
        // Emit connection state changed
        emit connectionStateChanged(false);

        // Only abort if we're not already disconnected
        if (m_webSocket.state() != QAbstractSocket::UnconnectedState)
        {
            this->m_webSocket.abort(); // Abort current socket
        }

        // Try to reconnect if enabled
        if (reconnects && !m_reconnectScheduled)
        {
            scheduleReconnect();
        }
    }
}

void WebSocketPubSub::onPing(const QByteArray &payload)
{
    qDebug() << "Received ping from server";
    // Update last message time
    m_lastMessageTime = QDateTime::currentDateTime();
    // Reset missed heartbeats counter
    m_missedHeartbeats = 0;
}

void WebSocketPubSub::onPong(const QByteArray &payload)
{
    qDebug() << "Received pong from server";
    // Update last message time
    m_lastMessageTime = QDateTime::currentDateTime();
    // Reset missed heartbeats counter
    m_missedHeartbeats = 0;

    // Check if the server is still reachable
    if (m_webSocket.state() != QAbstractSocket::ConnectedState)
    {
        qWarning() << "Server might be down!";
        onDisconnected(); // Manually trigger disconnect and reconnection logic
    }
}

// Heartbeat monitoring methods
void WebSocketPubSub::startHeartbeatMonitor()
{
    m_missedHeartbeats = 0;
    m_lastMessageTime = QDateTime::currentDateTime();

    // Start the heartbeat check timer
    if (!m_heartbeatTimer.isActive())
    {
        m_heartbeatTimer.start();
    }

    // Start the ping timer
    if (!m_pingTimer.isActive())
    {
        m_pingTimer.start();
    }
}

void WebSocketPubSub::stopHeartbeatMonitor()
{
    if (m_heartbeatTimer.isActive())
    {
        m_heartbeatTimer.stop();
    }

    if (m_pingTimer.isActive())
    {
        m_pingTimer.stop();
    }

    if (m_pingTimeoutTimer.isActive())
    {
        m_pingTimeoutTimer.stop();
    }

    // Clear pending pings
    m_pendingPings.clear();
}

void WebSocketPubSub::checkHeartbeat()
{
    // Only check heartbeat if we are supposed to be connected
    if (m_webSocket.state() != QAbstractSocket::ConnectedState)
    {
        return;
    }

    // Calculate time since last message
    qint64 msSinceLastMessage = m_lastMessageTime.msecsTo(QDateTime::currentDateTime());

    // If we haven't received any message for 2× the heartbeat interval
    if (msSinceLastMessage > (HEARTBEAT_INTERVAL * 2))
    {
        m_missedHeartbeats++;
        qDebug() << "Missed heartbeat #" << m_missedHeartbeats
                 << " (last message " << msSinceLastMessage << "ms ago)";

        // If we've missed too many heartbeats, consider the connection dead
        if (m_missedHeartbeats >= MAX_MISSED_HEARTBEATS)
        {
            qWarning() << "Too many missed heartbeats, connection appears dead";

            // Close and abort the socket to force a reconnection
            m_webSocket.abort();

            // Emit connection state changed
            emit connectionStateChanged(false);

            // Try to reconnect if enabled
            if (reconnects)
            {
                scheduleReconnect();
            }
        }
    }
    else
    {
        // Reset missed heartbeats if we're receiving messages
        m_missedHeartbeats = 0;
    }
}

void WebSocketPubSub::sendPing()
{
    // Only send ping if connected
    if (m_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "Sending ping to server";
        m_webSocket.ping();
    }
}

// reconnection logic
void WebSocketPubSub::scheduleReconnect()
{
    // Don't schedule multiple reconnects or try to reconnect when already connecting
    if (this->m_reconnectScheduled ||
        this->m_webSocket.state() == QAbstractSocket::ConnectingState ||
        this->m_webSocket.state() == QAbstractSocket::HostLookupState)
    {
        return;
    }

    this->m_reconnectScheduled = true;

    // Cap the reconnect attempts to prevent integer overflow
    const int maxAttempts = 10;
    if (this->m_reconnectAttempts >= maxAttempts)
    {
        this->m_reconnectAttempts = maxAttempts;
    }

    // Base delay is 1000ms (1 second)
    int baseDelay = 1000;
    int maxDelay = 30000; // 30 seconds max delay

    // Calculate delay with exponential backoff (1.5^n rather than 2^n for smoother growth)
    double factor = std::pow(1.5, this->m_reconnectAttempts);
    int delay = static_cast<int>(baseDelay * factor);

    // Add jitter (±20% randomness) to prevent reconnection storms
    int jitter = static_cast<int>(delay * 0.2 * (QRandomGenerator::global()->generateDouble() - 0.5));
    delay += jitter;

    // Ensure delay is within bounds (minimum 1s, maximum 30s)
    delay = qBound(1000, delay, maxDelay);

    qDebug() << "Reconnecting in" << delay << "ms (attempt" << (this->m_reconnectAttempts + 1) << ")";

    QTimer::singleShot(delay, this, [this]()
                       {
        this->m_reconnectScheduled = false;
        this->m_reconnectAttempts++;
        this->connectToUrl(this->m_lastUrl, this->reconnects); });
}