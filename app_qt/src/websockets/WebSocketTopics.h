#ifndef WEBSOCKETTOPICS_H
#define WEBSOCKETTOPICS_H

#include <stdexcept>
#include <QString>

enum WebSocketTopics
{
    GUI_READ_STATUS = 0,

    CM_STATUS = 1,
    CM_USER_MSG = 2,
    CM_PRODUCT_ACTIVE = 3,
    CM_PRODUCTS_DISABLED = 4,
};

QString topicToString(WebSocketTopics action)
{
    switch (action)
    {
    case GUI_READ_STATUS:
        return "GUI_READ_STATUS";
    case CM_STATUS:
        return "CM_STATUS";
    case CM_USER_MSG:
        return "CM_USER_MSG";
    case CM_PRODUCT_ACTIVE:
        return "CM_PRODUCT_ACTIVE";
    case CM_PRODUCTS_DISABLED:
        return "CM_PRODUCTS_DISABLED";
    default:
        qWarning() << "topicToString: UNKNOWN_TOPIC";
        return "UNKNOWN_TOPIC";
    }
}

#endif
