#include "WebSocketCmComs.h"
#include "WebSocketTopics.h"
#include "WebSocketAction.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

WebSocketCmComs::WebSocketCmComs(QObject *parent) : QObject(parent)
{
    this->m_ws = new WebSocketPubSub(this); // ownership tied to this
    connect(m_ws, &WebSocketPubSub::messageReceived, this, &WebSocketCmComs::onMessageReceived);
}

WebSocketCmComs::~WebSocketCmComs()
{
    // No need to delete if parent is set (Qt will clean it up)
}

void WebSocketCmComs::start(const QUrl &url, bool reconnect)
{
    this->m_ws->connectToUrl(url, reconnect);
}

void WebSocketCmComs::stop()
{
    this->m_ws->disconnect();
}

void WebSocketCmComs::onMessageReceived(const QString &action, const QString &topic, const QString &payload)
{
    qDebug() << "WebSocketCmComs::onMessageReceived(action=[" << action << "], topic=[" << topic << "], payload=[" << payload << "])";
    qDebug() << "Up to you to do something about it";

    if (action == actionToString(WebSocketAction::PUBLISH) && topic == topicToString(WebSocketTopics::GUI_READ_STATUS))
    {
        this->sendCmStatus();
    }
}

void WebSocketCmComs::subscribeToGui()
{
    this->m_ws->subscribe(topicToString(WebSocketTopics::GUI_READ_STATUS));
}

void WebSocketCmComs::sendCommand(const QString &topic, const QString &payload)
{
    this->m_ws->publish(topic, payload);
}

void WebSocketCmComs::onConnected()
{
    // TODO: something?
    qDebug() << "WebSocketCmComs::onConnected CONNECTED!";
}

void WebSocketCmComs::onError(QAbstractSocket::SocketError error)
{
    // TODO: something?
    qWarning() << "WebSocketCmComs::onError ERROR:" << error;
}

/**
 * COMMANDS
 */
void WebSocketCmComs::sendCmStatus()
{
    auto value = -1; // STATUS FOR MISISNG IMPL
    if (m_statusObserver)
    {
        value = m_statusObserver->getValue().toInt();
    }
    this->sendCmStatus(value);
}
void WebSocketCmComs::sendCmStatus(const int &status)
{
    this->m_ws->publish(topicToString(WebSocketTopics::CM_STATUS), QString::number(status));
}

void WebSocketCmComs::sendCmUserMsg(const int &userMessage)
{
    this->m_ws->publish(topicToString(WebSocketTopics::CM_USER_MSG), QString::number(userMessage));
}

void WebSocketCmComs::sendCmProductActive(const int &productId)
{
    this->m_ws->publish(topicToString(WebSocketTopics::CM_PRODUCT_ACTIVE), QString::number(productId));
}

void WebSocketCmComs::sendCmProductsDisabled(const QList<int> &productIds)
{
    QJsonArray jsonArray;
    for (int id : productIds)
        jsonArray.append(id);

    QJsonDocument doc(jsonArray);
    QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    this->m_ws->publish(topicToString(WebSocketTopics::CM_PRODUCTS_DISABLED), payload);
}

/**
 * STATE
 */
void WebSocketCmComs::setStatusObserver(QObject *observer)
{

    auto *atomObserver = dynamic_cast<IAtomObserver *>(observer);
    if (!atomObserver)
    {
        qWarning() << "Observer is not an IAtomObserver";
        return;
    }
    m_statusObserver = atomObserver;
}