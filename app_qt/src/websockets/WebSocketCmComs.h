#ifndef WEBSOCKETPCMCOMS_H
#define WEBSOCKETPCMCOMS_H

#include "WebSocketPubSub.h"

#include "../state/IAtomObserver.h"

class WebSocketCmComs : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketCmComs(QObject *parent = nullptr);
    ~WebSocketCmComs();

    Q_INVOKABLE void start(const QUrl &url, bool reconnect = true);
    Q_INVOKABLE void stop();

    Q_INVOKABLE void subscribeToGui();
    Q_INVOKABLE void sendCommand(const QString &topic, const QString &payload);

    // predefined commands
    Q_INVOKABLE void sendCmStatus();
    Q_INVOKABLE void sendCmStatus(const int &status);
    Q_INVOKABLE void sendCmUserMsg(const int &userMessage);
    Q_INVOKABLE void sendCmProductActive(const int &productId);
    Q_INVOKABLE void sendCmProductsDisabled(const QList<int> &productIds);

    // state
    Q_INVOKABLE void setStatusObserver(QObject *observer);

public slots:
    void onMessageReceived(const QString &action, const QString &topic, const QString &payload);
    void onConnected();
    void onError(QAbstractSocket::SocketError error);

signals:
    void messageReceived(const QString &action, const QString &topic, const QString &payload);

private:
    WebSocketPubSub *m_ws = nullptr;
    IAtomObserver *m_statusObserver = nullptr;
};

#endif // WEBSOCKETPCMCOMS_H
