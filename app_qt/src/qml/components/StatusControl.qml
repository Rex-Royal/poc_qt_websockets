// StatusControl.qml
import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    property var status: null // status property to be passed from parent
    property var label: "STATUS_CONTROLL"

    width: 300
    height: 50

    Row {
        spacing: 16
        anchors.left: parent.left // Align the Row to the left of its parent

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: label + ": " + status.value
            font.pointSize: 10
        }

        Button {
            anchors.verticalCenter: parent.verticalCenter
            text: "+"
            onClicked: {
                status.value += 1
            }
        }

        Button {
            anchors.verticalCenter: parent.verticalCenter
            text: "reset"
            onClicked: {
                status.value = 0
            }
        }

        Button {
            anchors.verticalCenter: parent.verticalCenter
            text: "-"
            onClicked: {
                status.value -= 1
            }
        }
    }
}
