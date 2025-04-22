import QtQuick 2.15
import QtQuick.Controls 2.15
import MyApp 1.0

ApplicationWindow {
    width: 640
    height: 360
    visible: true
    title: "WebSocket Pub/Sub"

    WebSocketPubSub {
        id: webSocketPubSub
        Component.onCompleted: {
            webSocketPubSub.connectToUrl("ws://localhost:3002")
        }

        // Handle incoming message for counter update
        onMessageReceived: function(topic, payload) {
        if (topic === "counter")
        {
            let value = parseInt(payload);
            if (!isNaN(value))
            {
                counter = value;
            }
        } else {
        console.log("Received:", topic, payload);
    }
}
}

// Maintain local state in QML
property int counter: 0

    Column {
        spacing: 10
        padding: 20

        width: parent.width

        TextField {
            id: topicInput
            width: parent.width
            placeholderText: "Enter topic"
        }

        Button {
            text: "Subscribe"
            onClicked: {
                webSocketPubSub.subscribe(topicInput.text)
            }
        }

        Button {
            text: "Subscribe to counter"
            onClicked: {
                webSocketPubSub.subscribe("counter")
                topicInput.text = "counter"
            }
        }

        Button {
            text: "Unsubscribe"
            onClicked: {
                webSocketPubSub.unsubscribe(topicInput.text)
            }
        }

        Button {
            text: "Reset"
            onClicked: {
                webSocketPubSub.publish(topicInput.text, "0")
            }
        }

        Text {
            id: counterText
            text: "Counter: " + counter
            font.pointSize: 24
        }

        Row {
            spacing: 10
            Button {
                text: "Increment"
                onClicked: {
                    webSocketPubSub.publish("counter", counter + 1)
                }
            }
            Button {
                text: "Decrement"
                onClicked: {
                    webSocketPubSub.publish("counter", counter - 1)
                }
            }
        }

        ListView {
            width: parent.width
            height: 100
            model: webSocketPubSub

            delegate: Text {
                text: modelData  // Display the topic name
            }
        }
    }
}
