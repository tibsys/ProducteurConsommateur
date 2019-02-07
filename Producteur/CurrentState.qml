import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import ProducteurConsommateur 1.0

Rectangle {
    id: currentState
    width: 50
    height: 50
    states: [
        State {
            name: "Started"

            PropertyChanges {
                target: currentState
                color: "#9eed00"
            }
        },
        State {
            name: "Stopped"

            PropertyChanges {
                target: currentState
                color: "#e64238"
            }
        },
        State {
            name: "Connecting"

            PropertyChanges {
                target: currentState
                color: "#fdce2c"
            }
        }
    ]
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: buttonStart.verticalCenter
}
