import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtCharts 2.2
import ProducteurConsommateur 1.0

Window {
    id: producteurWindow
    visible: true
    width: 800
    height: 480
    title: qsTr("Producteur")

    Producteur {
        id: producteur
        frameRate: slider.value
        realTime: realTime.checked
        onConnectingToConsommateur: currentState.state = "Connecting";
        onConnectedToConsommateur: {
            currentState.state = "Started";
            timerRefreshIndicators.start()
            seriesEffectiveRate.clear()
            seriesMissed.clear()
            seriesRate.clear()
        }
        onTraitementStoppe: {
            currentState.state = "Stopped"
            timerRefreshIndicators.stop()
        }
    }

    Timer {
        id: timerRefreshIndicators
        interval: 500
        repeat: true
        onTriggered: {
            //Met à jour les indicateurs
            lblTramesEmises.text = producteur.sentFrames
            lblTramesPerdues.text = producteur.missedFrames
            lblEffectiveRate.text = producteur.effectiveRate
            var framerateText = "(%1%)"
            lblLostFramesRate.text = framerateText.arg( Math.round(producteur.missedFrames*100 / producteur.sentFrames) )

            //Met à jour le graphique
            seriesRate.append(producteur.sentFrames, slider.value)
            seriesMissed.append(producteur.sentFrames, producteur.missedFrames)
            seriesEffectiveRate.append(producteur.sentFrames, producteur.effectiveRate)

            if(producteur.sentFrames > 70) {
                axisX.min = producteur.sentFrames - 70
                axisX.max = producteur.sentFrames + 30
            }
        }
    }

    Text {
        id: title
        x: 25
        y: 13
        text: qsTr("Producteur")
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 20
    }

    Text {
        id: titleRate
        x: 21
        y: 57
        text: qsTr("Débit (fps) :")
        anchors.verticalCenter: slider.verticalCenter
        anchors.right: slider.left
        anchors.rightMargin: 11
        font.pixelSize: 12
    }

    Slider {
        id: slider
        anchors.left: title.right
        anchors.leftMargin: -150
        stepSize: 1
        to: 100
        from: 1
        anchors.top: title.bottom
        anchors.topMargin: 20
        value: 1
    }

    Text {
        id: selectedRate
        x: 20
        y: 70
        width: 20
        horizontalAlignment: Text.AlignLeft
        anchors.left: slider.right
        anchors.leftMargin: 10
        font.pixelSize: 12
        text: slider.value
    }

    Button {
        id: buttonStart
        x: 152
        y: 390
        text: qsTr("Démarrer")
        anchors.right: currentState.left
        anchors.rightMargin: 25
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        onClicked: {
            producteur.connectToConsommateur();
        }
    }

    Button {
        id: buttonStop
        y: 390
        text: qsTr("Arrêter")
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        anchors.left: currentState.right
        anchors.leftMargin: 25
        onClicked: {
            producteur.stop()
        }
    }

    GroupBox {
        id: groupBox
        width: 190
        anchors.top: slider.bottom
        anchors.topMargin: 50
        anchors.bottom: buttonStart.top
        anchors.bottomMargin: 50
        anchors.left: parent.left
        anchors.leftMargin: 20
        title: qsTr("Indicateurs")

        Text {
            id: element
            x: 6
            y: 0
            text: qsTr("Trames émises :")
            font.pixelSize: 12
        }

        Text {
            id: lblTramesEmises
            x: 97
            y: 6
            text: qsTr("0")
            anchors.verticalCenter: element.verticalCenter
            font.pixelSize: 12
        }

        Text {
            id: element1
            x: 6
            y: 20
            text: qsTr("Trames perdues :")
            font.pixelSize: 12
        }

        Text {
            id: lblTramesPerdues
            x: 104
            y: 20
            text: qsTr("0")
            anchors.verticalCenter: element1.verticalCenter
            font.pixelSize: 12
        }

        Text {
            id: element2
            x: 6
            y: 40
            text: qsTr("Débit effectif :")
            font.pixelSize: 12
        }

        Text {
            id: lblEffectiveRate
            x: 89
            y: 40
            text: qsTr("0")
            font.pixelSize: 12
        }

        Text {
            id: element3
            x: 102
            y: 40
            text: qsTr("fps")
            anchors.verticalCenter: lblEffectiveRate.verticalCenter
            font.pixelSize: 12
        }

        Text {
            id: lblLostFramesRate
            y: 20
            text: qsTr("(0%)")
            anchors.left: lblTramesPerdues.right
            anchors.leftMargin: 6
            anchors.verticalCenter: lblTramesPerdues.verticalCenter
            font.pixelSize: 12
        }
    }

    CurrentState {
        id: currentState
        x: 181
        y: 363
        anchors.verticalCenter: buttonStart.verticalCenter
    }

    ChartView {
        id: graphView
        anchors.left: groupBox.right
        anchors.leftMargin: 10
        anchors.bottom: buttonStop.top
        anchors.bottomMargin: 0
        anchors.top: slider.bottom
        anchors.topMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 20
        antialiasing: true

        LineSeries {
            id: seriesRate
            name: "Débit ciblé (fps)"
            color: "#000000"
            axisX: axisX
            axisY: axisY
        }

        LineSeries {
            id: seriesMissed
            name: "Perdues"
            color: "#FF0000"
            axisX: axisX
            axisY: axisY
        }

        LineSeries {
            id: seriesEffectiveRate
            name: "Débit effectif (fps)"
            color: "#0000FF"
            axisX: axisX
            axisY: axisY
        }

        ValueAxis {
            id: axisX
            color: "#000000"
            min: 0
            max: 100
        }

        ValueAxis {
            id: axisY
            color: "#000000"
            min: 0
            max: 100
        }
    }

    CheckBox {
        id: realTime
        y: 51
        text: qsTr("Temps réel")
        anchors.verticalCenter: selectedRate.verticalCenter
        anchors.left: selectedRate.right
        anchors.leftMargin: 10
    }

    Component.onCompleted: {

    }
}



























































/*##^## Designer {
    D{i:1;anchors_x:25}D{i:2;anchors_y:57}D{i:3;anchors_x:98;anchors_y:44}D{i:4;anchors_x:304}
D{i:6;anchors_x:152}D{i:7;anchors_x:219}D{i:8;anchors_height:200;anchors_width:200;anchors_x:31;anchors_y:120}
D{i:17;anchors_x:117}D{i:19;anchors_height:200;anchors_width:200;anchors_x:786;anchors_y:230}
D{i:25;anchors_x:541}
}
 ##^##*/
