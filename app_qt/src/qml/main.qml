import QtQuick 2.15
import QtQuick.Controls 2.15
import MyApp 1.0

import "components"

ApplicationWindow {
    width: 1600
    height: 360
    visible: true
    title: "WebSocket Pub/Sub"

    property int counter: 0

    WebSocketCmComs  {
        id: webSocketCmComs

        onMessageReceived: function(action, topic, payload) {
            // console.log("on message received");
            if (topic === "counter") {
                let value = parseInt(payload);
                if (!isNaN(value)) {
                    counter = value;
                }
            } else if (topic === "GUI_READ_STATUS") {
                // response on "GUI read status" request
                // webSocketCmComs.sendCmStatus("CM_STATUS", 2);
            } else {
                console.log("Received:", action, " >> ", topic, " - ", payload);
            }
        }
    }

    Timer {
        id: subscriptionDelay
        interval: 300
        repeat: false
        onTriggered: {
            console.log("Subscribing to GUI_READ_STATUS");
            webSocketCmComs.subscribeToGui();
        }
    }

    AtomObserverWrapper {
        id: cm_status_obs
        atom: cm_status

        Component.onCompleted: {
            // observer to trigger websocket "Send CM Status" on status state change
            cm_status_obs.observe(() => {
                console.log("CM STATUS observed from QML: ", cm_status_obs.getValue());
                webSocketCmComs.sendCmStatus()
            });
        }
    }

    Component.onCompleted: {
        webSocketCmComs.setStatusObserver(cm_status_obs)
        webSocketCmComs.start(wsUrl);  // connect to WebSocket server
        subscriptionDelay.start();
    }

    Atom {
        id: cm_status
        value: 2
    }
    Atom {
        id: cm_user_msg
        value: 4
    }
    Atom {
        id: cm_product_active
        value: 8
    }
    Atom {
        id: cm_products_disabled
        value: [16, 32, 64]
    }

    

    Row {
        anchors.fill: parent
        spacing: 40
        padding: 20

        Column {
            spacing: 10
            width: parent.width * 1/3

            Button {
                text: "CM_STATUS"
                onClicked: {
                    webSocketCmComs.sendCmStatus()
                }
            }
            Button {
                text: "CM_USER_MSG"
                onClicked: {
                    // webSocketCmComs
                    webSocketCmComs.sendCmUserMsg(cm_user_msg.value)
                }
            }
            Button {
                text: "CM_PRODUCT_ACTIVE"
                onClicked: {
                    // webSocketCmComs
                    webSocketCmComs.sendCmProductActive(cm_product_active.value)
                }
            }
            Button {
                text: "CM_PRODUCTS_DISABLED"
                onClicked: {
                    // webSocketCmComs
                    webSocketCmComs.sendCmProductsDisabled(cm_products_disabled.value)
                }
            }
        }

        Column {
            spacing: 10
            width: parent.width * 2/3

            StatusControl {
                label: "STATUS"
                status: cm_status
            }

            StatusControl {
                label: "USER MSG (not auto)"
                status: cm_user_msg
            }

            StatusControl {
                label: "PRODUCT ACTIVE (not auto)"
                status: cm_product_active
            }

            StatusControlArray {
                label: "PRODUCTS DISABLED (not auto)"
                status: cm_products_disabled
            }
        }
    }
}
