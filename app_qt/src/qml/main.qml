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
        onMessageReceived: function(topic, message) {
            console.log("TOPIC: ["+topic+"] MESSAGE: [" + message + "]");

            if (topic === "counter") {
                let value = parseInt(message);
                if (!isNaN(value)) {
                    counter = value;
                }
            } else {
                console.log("Received:", topic, message);
            }
        }
    }

    // Maintain local state in QML
    property int counter: 0
    property var subscribedTopics: []

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
                subscribedTopics.push(topicInput.text)
            }
        }

        Button {
            text: "Unsubscribe"
            onClicked: {
                webSocketPubSub.unsubscribe(topicInput.text)
                var index = subscribedTopics.indexOf(topicInput.text)
                if (index >= 0) subscribedTopics.splice(index, 1)
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
            model: subscribedTopics

            delegate: Text {
                text: modelData  // Display the topic name
            }
        }
    }
}
