import QtQuick 2.15
import QtQuick.Controls 2.15
import MyApp 1.0

ApplicationWindow {
    width: 640
    height: 360
    visible: true
    title: "WebSocket Pub/Sub"

    property int counter: 0

    WebSocketPubSub {
        id: webSocketPubSub

        onMessageReceived: function(topic, payload) {
            if (topic === "counter") {
                let value = parseInt(payload);
                if (!isNaN(value)) {
                    counter = value;
                }
            } else if (topic === "GUI_READ_STATUS") {
                webSocketPubSub.publish("CM_STATUS", "2")
            } else {
                console.log("Received:", topic, payload);
            }
        }
    }

    Timer {
        id: subscriptionDelay
        interval: 300
        repeat: false
        onTriggered: {
            console.log("Subscribing to GUI_READ_STATUS");
            webSocketPubSub.subscribe("GUI_READ_STATUS");
        }
    }

    Component.onCompleted: {
        webSocketPubSub.connectToUrl("ws://localhost:3002");
        subscriptionDelay.start();
    }

    Row {
        anchors.fill: parent
        spacing: 40
        padding: 20

        Column {
            spacing: 10
            width: parent.width / 2

            TextField {
                id: topicInput
                width: parent.width
                placeholderText: "Enter topic"
            }

            Button {
                text: "Subscribe"
                onClicked: webSocketPubSub.subscribe(topicInput.text)
            }

            Button {
                text: "Subscribe to counter"
                onClicked: {
                    webSocketPubSub.subscribe("counter");
                    topicInput.text = "counter";
                }
            }

            Button {
                text: "Unsubscribe"
                onClicked: webSocketPubSub.unsubscribe(topicInput.text)
            }

            Button {
                text: "Reset"
                onClicked: webSocketPubSub.publish(topicInput.text, "0")
            }

            Text {
                text: "Counter: " + counter
                font.pointSize: 24
            }

            Row {
                spacing: 10
                Button {
                    text: "Increment"
                    onClicked: webSocketPubSub.publish("counter", counter + 1)
                }
                Button {
                    text: "Decrement"
                    onClicked: webSocketPubSub.publish("counter", counter - 1)
                }
            }
        }

        Column {
            spacing: 10
            width: parent.width / 2

            Button {
                text: "CM_STATUS"
                onClicked: webSocketPubSub.publish("CM_STATUS", "2")
            }
            Button {
                text: "CM_USER_MSG"
                onClicked: webSocketPubSub.publish("CM_USER_MSG", "3")
            }
            Button {
                text: "CM_PRODUCT_ACTIVE"
                onClicked: webSocketPubSub.publish("CM_PRODUCT_ACTIVE", "4")
            }
            Button {
                text: "CM_PRODUCTS_DISABLED"
                onClicked: webSocketPubSub.publish("CM_PRODUCTS_DISABLED", "[7, 9, 16]")
            }
        }
    }
}
