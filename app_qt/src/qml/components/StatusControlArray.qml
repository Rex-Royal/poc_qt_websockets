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
        // anchors.centerIn: parent

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: label + ": " + status.value
            font.pointSize: 10
        }

        Text {
            text: "Value at index 0: " + cm_products_disabled.value[0]
        }

        Button {
            anchors.verticalCenter: parent.verticalCenter
            text: "[0]+"
            onClicked: {
                var newValue = status.value
                newValue[0] += 1
                status.value = newValue
            }
        }

        Button {
            anchors.verticalCenter: parent.verticalCenter
            text: "reset"
            onClicked: {
                status.value = [0, 32, 64]
            }
        }

        Button {
            anchors.verticalCenter: parent.verticalCenter
            text: "[0]-"
            onClicked: {
                var newValue = status.value
                newValue[0] -= 1
                status.value = newValue
            }
        }
    }
}
