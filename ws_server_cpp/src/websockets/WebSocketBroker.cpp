#include "WebSocketBroker.h"
#include <QDateTime>
#include <QUuid>
#include <QJsonArray>
#include <QThread>
#include <QApplication>
#include <QCoreApplication>
#include <QNetworkInterface>
#include <QtMath>
#include <QRandomGenerator>

// Initialize static instance
WebSocketBroker *WebSocketBroker::instance = nullptr;

WebSocketBroker::WebSocketBroker(int port, QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer(QStringLiteral("WebSocket Broker"),
                                    QWebSocketServer::NonSecureMode, this)),
      m_healthCheckTimer(new QTimer(this)),
      m_recoveryTimer(new QTimer(this)),
      m_port(port),
      m_recoveryAttempts(0),
      m_isRecovering(false)
{
    // Setup health check timer (runs every 30 seconds)
    m_healthCheckTimer->setInterval(30000);
    connect(m_healthCheckTimer, &QTimer::timeout, this, &WebSocketBroker::performHealthCheck);
    m_healthCheckTimer->start();

    // Configure and start the server
    start();
}

WebSocketBroker *WebSocketBroker::getInstance(int port)
{
    if (!instance)
    {
        instance = new WebSocketBroker(port);
    }
    return instance;
}

WebSocketBroker::~WebSocketBroker()
{
    stop();
}

bool WebSocketBroker::start()
{
    if (m_server->isListening())
    {
        return true;
    }

    // Setup signal connections
    connect(m_server, &QWebSocketServer::newConnection,
            this, &WebSocketBroker::onNewConnection);
    connect(m_server, &QWebSocketServer::serverError,
            this, &WebSocketBroker::onServerError);

    // Start the server
    bool success = m_server->listen(QHostAddress::Any, m_port);
    if (success)
    {
        qDebug().noquote() << QString("✅ WebSocket Broker listening on ws://localhost:%1").arg(m_server->serverPort());
        m_recoveryAttempts = 0;
        m_isRecovering = false;
    }
    else
    {
        qDebug().noquote() << QString("❌ Failed to start WebSocket Broker on port %1: %2")
                        .arg(m_port)
                        .arg(m_server->errorString());
        scheduleServerRecovery();
    }

    return success;
}

bool WebSocketBroker::stop()
{
    if (!m_server->isListening())
    {
        return true;
    }

    m_healthCheckTimer->stop();
    m_recoveryTimer->stop();

    // Disconnect all clients
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        QWebSocket *socket = it.key();
        socket->close(QWebSocketProtocol::CloseCodeNormal, "Server shutting down");
    }

    // Clear client list
    m_clients.clear();

    // Close the server
    m_server->close();
    qDebug().noquote() << "WebSocket Broker stopped";

    return true;
}

void WebSocketBroker::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();

    // Create client info structure
    ClientInfo clientInfo;
    clientInfo.clientId = generateClientId();
    clientInfo.address = socket->peerAddress();
    clientInfo.port = socket->peerPort();

    // Add client to map
    m_clients.insert(socket, clientInfo);

    // Connect socket signals
    connect(socket, &QWebSocket::textMessageReceived,
            this, &WebSocketBroker::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected,
            this, &WebSocketBroker::onClientDisconnected);

    qDebug().noquote() << QString("🔌 Client connected: %1:%2 (ID: %3)")
                    .arg(clientInfo.address.toString())
                    .arg(clientInfo.port)
                    .arg(clientInfo.clientId);

    // Send welcome message to client
    QJsonObject welcomeMsg;
    welcomeMsg["action"] = actionToString(WebSocketAction::PUBLISH);
    welcomeMsg["topic"] = "system";
    welcomeMsg["payload"] = QString("Welcome! Your client ID is %1").arg(clientInfo.clientId);
    socket->sendTextMessage(QJsonDocument(welcomeMsg).toJson(QJsonDocument::Compact));
}

QString getPrettyMessage(QString message)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    QString prettyMessage;
    if (!doc.isNull() && parseError.error == QJsonParseError::NoError)
    {
        return doc.toJson(QJsonDocument::Indented).trimmed(); // Pretty-print
    }
    return message; // Fallback to raw message if not valid JSON
}

void WebSocketBroker::onTextMessageReceived(const QString &message)
{
    QWebSocket *senderSocket = qobject_cast<QWebSocket *>(sender());
    if (senderSocket && m_clients.contains(senderSocket))
    {
        const ClientInfo &clientInfo = m_clients[senderSocket];
        qDebug().noquote() << QString("📨 Message from %1:%2 (ID: %3): %4")
                                  .arg(clientInfo.address.toString())
                                  .arg(clientInfo.port)
                                  .arg(clientInfo.clientId)
                                  .arg(getPrettyMessage(message));

        handleMessage(message, senderSocket);
    }
}

void WebSocketBroker::onClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (client && m_clients.contains(client))
    {
        const ClientInfo &clientInfo = m_clients[client];
        qDebug().noquote() << QString("👋 Client disconnected: %1:%2 (ID: %3)")
                        .arg(clientInfo.address.toString())
                        .arg(clientInfo.port)
                        .arg(clientInfo.clientId);

        // Remove client
        m_clients.remove(client);
        client->deleteLater();
    }
}

void WebSocketBroker::onServerError(QWebSocketProtocol::CloseCode closeCode)
{
    qDebug().noquote() << QString("⚠️ WebSocket server error: %1 (code: %2)")
                    .arg(m_server->errorString())
                    .arg(closeCode);

    // Try to recover
    scheduleServerRecovery();
}

void WebSocketBroker::performHealthCheck()
{
    if (!m_server->isListening() && !m_isRecovering)
    {
        qDebug().noquote() << "🏥 Health check detected server is down, attempting recovery";
        scheduleServerRecovery();
        return;
    }

    int clientCount = m_clients.size();
    qDebug().noquote() << QString("🏥 Health check: Server is running with %1 client(s) connected").arg(clientCount);

    // Optional: Ping clients to verify they're still responsive
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        QWebSocket *socket = it.key();
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            // Send a ping to keep connection alive
            socket->ping();
        }
    }
}

void WebSocketBroker::scheduleServerRecovery()
{
    if (m_isRecovering)
    {
        return;
    }

    m_isRecovering = true;

    // Exponential backoff with jitter
    // Base delay is 1000ms (1 second)
    int baseDelay = 1000;
    int maxDelay = 30000; // 30 seconds max delay

    // Cap the recovery attempts
    const int maxAttempts = 10;
    if (m_recoveryAttempts >= maxAttempts)
    {
        m_recoveryAttempts = maxAttempts;
    }

    // Calculate delay with exponential backoff
    double factor = qPow(1.5, m_recoveryAttempts);
    int delay = static_cast<int>(baseDelay * factor);

    // Add jitter (±20% randomness)
    int jitter = static_cast<int>(delay * 0.2 * (QRandomGenerator::global()->generateDouble() - 0.5));
    delay += jitter;

    // Ensure delay is within bounds
    delay = qBound(1000, delay, maxDelay);

    qDebug().noquote() << QString("🔄 Scheduling server recovery attempt #%1 in %2 ms")
                    .arg(m_recoveryAttempts + 1)
                    .arg(delay);

    // Stop any existing recovery timer
    if (m_recoveryTimer->isActive())
    {
        m_recoveryTimer->stop();
    }

    // Schedule recovery attempt
    m_recoveryTimer->setSingleShot(true);
    m_recoveryTimer->setInterval(delay);
    connect(m_recoveryTimer, &QTimer::timeout, this, &WebSocketBroker::attemptServerRecovery, Qt::UniqueConnection);
    m_recoveryTimer->start();
}

void WebSocketBroker::attemptServerRecovery()
{
    qDebug().noquote() << QString("🔄 Attempting server recovery #%1").arg(m_recoveryAttempts + 1);

    // Try to restart the server
    if (m_server->isListening())
    {
        m_server->close();
    }

    // Disconnect all signal connections to create a clean slate
    disconnect(m_server, &QWebSocketServer::newConnection, this, &WebSocketBroker::onNewConnection);
    disconnect(m_server, &QWebSocketServer::serverError, this, &WebSocketBroker::onServerError);

    // Try with a different port if needed after several attempts
    if (m_recoveryAttempts > 3)
    {
        int alternativePort = m_port + (m_recoveryAttempts % 10);
        qDebug().noquote() << QString("🔄 Trying alternative port: %1").arg(alternativePort);
        m_port = alternativePort;
    }

    // Attempt to start the server
    if (start())
    {
        qDebug().noquote() << "✅ Server recovery successful!";
        m_isRecovering = false;
        m_recoveryAttempts = 0;
    }
    else
    {
        m_recoveryAttempts++;
        m_isRecovering = false; // This allows scheduleServerRecovery to be called again
        scheduleServerRecovery();
    }
}

void WebSocketBroker::publish(const QString &topic, const QString &payload, QWebSocket *excludeClient)
{
    QJsonObject message;
    message["action"] = actionToString(WebSocketAction::PUBLISH);
    message["topic"] = topic;
    message["payload"] = payload;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QString messageStr = QJsonDocument(message).toJson(QJsonDocument::Compact);
    int sentCount = 0;

    // Send to all clients subscribed to this topic
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        QWebSocket *socket = it.key();
        const ClientInfo &clientInfo = it.value();

        // Skip the excluded client if specified
        if (excludeClient && socket == excludeClient)
        {
            continue;
        }

        // Send to clients subscribed to this topic
        if (clientInfo.topics.contains(topic))
        {
            socket->sendTextMessage(messageStr);
            sentCount++;
        }
    }

    qDebug().noquote() << QString("📤 Published to topic '%1': %2 (Delivered to %3 client(s))")
                              .arg(topic)
                              .arg(payload)
                              .arg(sentCount);
}

void WebSocketBroker::broadcast(const QString &payload, QWebSocket *excludeClient)
{
    QJsonObject message;
    message["action"] = actionToString(WebSocketAction::BROADCAST);
    message["payload"] = payload;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QString messageStr = QJsonDocument(message).toJson(QJsonDocument::Compact);
    int sentCount = 0;

    // Send to all connected clients
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        QWebSocket *socket = it.key();

        // Skip the excluded client if specified
        if (excludeClient && socket == excludeClient)
        {
            continue;
        }

        socket->sendTextMessage(messageStr);
        sentCount++;
    }

    qDebug().noquote() << QString("📢 Broadcast: %1 (Delivered to %2 client(s))")
                              .arg(payload)
                              .arg(sentCount);
}

bool WebSocketBroker::isRunning() const
{
    return m_server->isListening();
}

int WebSocketBroker::serverPort() const
{
    return m_server->serverPort();
}

QString WebSocketBroker::actionToString(WebSocketAction action)
{
    switch (action)
    {
    case WebSocketAction::SUBSCRIBE:
        return "subscribe";
    case WebSocketAction::UNSUBSCRIBE:
        return "unsubscribe";
    case WebSocketAction::PUBLISH:
        return "publish";
    case WebSocketAction::BROADCAST:
        return "broadcast";
    default:
        return "unknown";
    }
}

WebSocketAction WebSocketBroker::stringToAction(const QString &action)
{
    QString actionLower = action.toLower();

    if (actionLower == "subscribe")
        return WebSocketAction::SUBSCRIBE;
    if (actionLower == "unsubscribe")
        return WebSocketAction::UNSUBSCRIBE;
    if (actionLower == "publish")
        return WebSocketAction::PUBLISH;
    if (actionLower == "broadcast")
        return WebSocketAction::BROADCAST;

    // Default to publish if unknown
    return WebSocketAction::PUBLISH;
}

QString WebSocketBroker::generateClientId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
}

void WebSocketBroker::handleMessage(const QString &message, QWebSocket *client)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject())
    {
        qDebug().noquote() << "Invalid message format (not a JSON object)";
        return;
    }

    QJsonObject jsonObj = doc.object();

    // Extract message fields
    if (!jsonObj.contains("action"))
    {
        qDebug().noquote() << "Message missing 'action' field";
        return;
    }

    QString actionStr = jsonObj["action"].toString();
    WebSocketAction action = stringToAction(actionStr);

    // Handle different message types
    switch (action)
    {
    case WebSocketAction::SUBSCRIBE:
    {
        if (!jsonObj.contains("topic"))
        {
            qDebug().noquote() << "Subscribe message missing 'topic' field";
            return;
        }

        QString topic = jsonObj["topic"].toString();
        m_clients[client].topics.insert(topic);

        qDebug().noquote() << QString("👂 Client %1 subscribed to topic '%2'")
                                  .arg(m_clients[client].clientId)
                                  .arg(topic);

        // Send confirmation
        QJsonObject response;
        response["action"] = "subscribe";
        response["status"] = "success";
        response["topic"] = topic;
        client->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
        break;
    }

    case WebSocketAction::UNSUBSCRIBE:
    {
        if (!jsonObj.contains("topic"))
        {
            qDebug().noquote() << "Unsubscribe message missing 'topic' field";
            return;
        }

        QString topic = jsonObj["topic"].toString();
        m_clients[client].topics.remove(topic);

        qDebug().noquote() << QString("🙉 Client %1 unsubscribed from topic '%2'")
                        .arg(m_clients[client].clientId)
                        .arg(topic);

        // Send confirmation
        QJsonObject response;
        response["action"] = "unsubscribe";
        response["status"] = "success";
        response["topic"] = topic;
        client->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
        break;
    }

    case WebSocketAction::PUBLISH:
    {
        if (!jsonObj.contains("topic") || !jsonObj.contains("payload"))
        {
            qDebug().noquote() << "Publish message missing 'topic' or 'payload' field";
            return;
        }

        QString topic = jsonObj["topic"].toString();
        QString payload = jsonObj["payload"].toString();

        // Republish to all subscribers (excluding sender to avoid echo)
        publish(topic, payload, client);
        break;
    }

    case WebSocketAction::BROADCAST:
    {
        if (!jsonObj.contains("payload"))
        {
            qDebug().noquote() << "Broadcast message missing 'payload' field";
            return;
        }

        QString payload = jsonObj["payload"].toString();

        // Broadcast to all clients (excluding sender to avoid echo)
        broadcast(payload, client);
        break;
    }

    default:
        qDebug().noquote() << "Unknown action: " << actionStr;
        break;
    }
}
