import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import ProducteurConsommateur 1.0

Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("Consommateur")

    Consommateur
    {
        id: consommateur
        onServerError: {
            console.log("Erreur d'ouverture de la socket")
        }
        treatmentDuration: slider.value
    }

    Timer {
        id: timerRefreshIndicators
        interval: 500
        repeat: true
        onTriggered: {
            lblTramesRecues.text = consommateur.receivedFrames
            lblReponsesEnvoyees.text = consommateur.sentAck
        }
    }

    Text {
        id: title
        x: 25
        y: 13
        text: qsTr("Consommateur")
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 20
    }

    Text {
        id: titleTreatmentDuration
        x: 21
        y: 57
        text: qsTr("Durée du traitement (ms) :")
        anchors.verticalCenter: slider.verticalCenter
        anchors.right: slider.left
        anchors.rightMargin: 11
        font.pixelSize: 12
    }

    Slider {
        id: slider
        anchors.horizontalCenter: parent.horizontalCenter
        stepSize: 50
        to: 1000
        from: 1
        anchors.top: comboBox.bottom
        anchors.topMargin: 20
        value: 50
    }

    GroupBox {
        id: groupBox
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.top: slider.bottom
        anchors.topMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 20
        title: qsTr("Indicateurs")

        Text {
            id: element
            x: 5
            y: 0
            text: qsTr("Trames reçues :")
            font.pixelSize: 12
        }

        Text {
            id: lblTramesRecues
            y: 0
            text: "0"
            anchors.verticalCenter: element.verticalCenter
            anchors.left: element.right
            anchors.leftMargin: 6
            font.pixelSize: 12
        }

        Text {
            id: element1
            x: 5
            y: 20
            text: qsTr("Réponses envoyées :")
            font.pixelSize: 12
        }

        Text {
            id: lblReponsesEnvoyees
            y: 20
            text: qsTr("0")
            anchors.verticalCenter: element1.verticalCenter
            anchors.left: element1.right
            anchors.leftMargin: 6
            font.pixelSize: 12
        }
    }

    Text {
        id: element2
        y: 70
        anchors.left: slider.right
        anchors.leftMargin: 12
        anchors.verticalCenter: slider.verticalCenter
        font.pixelSize: 12
        text: slider.value
    }

    Text {
        id: element3
        x: 71
        y: 57
        text: qsTr("Mode :")
        anchors.verticalCenter: comboBox.verticalCenter
        anchors.right: titleTreatmentDuration.right
        anchors.rightMargin: 0
        font.pixelSize: 12
    }

    ComboBox {
        id: comboBox
        x: 226
        anchors.top: title.bottom
        anchors.topMargin: 30
        model: ["Synchrone", "Asynchrone", "Producteur-Consommateur"]
        onCurrentIndexChanged: {
            switch(comboBox.currentIndex) {
                case 0: consommateur.setModeSynchrone(); break;
                case 1: consommateur.setModeASynchrone(); break;
                case 2: consommateur.setModeProducteurConsommateur(); break;
                default: consommateur.setModeSynchrone(); break;
            }
        }
    }

    Component.onCompleted: {
        consommateur.startServer()
        console.log("Consommateur démarré")

        timerRefreshIndicators.start()
    }

}









/*##^## Designer {
    D{i:4;anchors_y:57}D{i:5;anchors_height:200;anchors_width:200;anchors_x:30;anchors_y:123}
D{i:8;anchors_x:95}D{i:10;anchors_x:122}D{i:11;anchors_x:432}D{i:12;anchors_x:71;anchors_y:57}
D{i:13;anchors_y:57}
}
 ##^##*/
