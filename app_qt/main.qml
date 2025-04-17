import QtQuick 2.15
import QtQuick.Controls 2.15
import QtWebSockets 1.15  // Import the QtWebSockets module

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "WebSocket Example"

    Rectangle {
        width: 640
        height: 480
        color: "lightblue"

        // WebSocket object
        WebSocket {
            id: websocket
            url: "ws://echo.websocket.org"  // Example WebSocket URL (you can replace it)
            onTextMessageReceived: {
                console.log("Received message: " + message)
                messageLabel.text = "Received: " + message
            }
            onStatusChanged: {
                if (websocket.status === WebSocket.Open) {
                    console.log("WebSocket connected")
                    websocket.sendTextMessage("Hello, WebSocket!")
                } else {
                    console.log("WebSocket status: " + websocket.status)
                }
            }
        }

        // Text to show WebSocket message
        Text {
            id: messageLabel
            anchors.centerIn: parent
            text: "Waiting for WebSocket message..."
            font.pixelSize: 24
        }
    }
}
