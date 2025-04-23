#include "WebSocketAction.h"
#include <QDebug>

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
        qWarning() << "topicToString: UNKNOWN_TOPIC";
        return "UNKNOWN_TOPIC";
    }
}