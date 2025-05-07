#ifndef WEBSOCKETTOPICS_H
#define WEBSOCKETTOPICS_H

#include <stdexcept>
#include <QString>

enum WebSocketTopics
{
    GUI_READ_STATUS = 0,
    GUI_PRODUCT_START = 1,
    GUI_PRODUCT_STOP = 2,
    GUI_MSG_COMMAND = 3,

    CM_STATUS = 4,
    CM_USER_MSG = 5,
    CM_PRODUCT_ACTIVE = 6,
    CM_PRODUCTS_DISABLED = 7,
};

QString topicToString(WebSocketTopics action)
{
    switch (action)
    {
    case GUI_READ_STATUS:
        return "GUI_READ_STATUS";
    case GUI_PRODUCT_START:
        return "GUI_PRODUCT_START";
    case GUI_PRODUCT_STOP:
        return "GUI_PRODUCT_STOP";
    case GUI_MSG_COMMAND:
        return "GUI_MSG_COMMAND";
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
