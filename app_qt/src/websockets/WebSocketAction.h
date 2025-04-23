#ifndef WEBSOCKETACTION_H
#define WEBSOCKETACTION_H

#include <QString>

enum WebSocketAction
{
    SUBSCRIBE = 0,
    PUBLISH = 1,
    UNSUBSCRIBE = 2
};

QString actionToString(WebSocketAction action);

#endif
