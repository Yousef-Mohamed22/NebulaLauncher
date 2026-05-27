import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {

    property NblInstallationRef installation

    width: 180 + 16
    height: 270 + 16

    Image {
        id: image
        source: "file://" + ApplicationContext.dataFolder + "/media/poster/" + installation.poster
        anchors.centerIn: parent
        width: parent.width - 16
        height: parent.height - 16
        visible: false
        layer.enabled: true
    }

    Item {
        id: maskShape
        width: image.width
        height: image.height
        visible: false
        layer.enabled: true
        Rectangle {
            anchors.fill: parent
            radius: 8
        }
    }

    MultiEffect {
        source: image
        anchors.fill: image
        maskEnabled: true
        maskSource: maskShape

        scale: mouseArea.containsMouse ? 1.05 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            ApplicationContext.launchInstallation(installation.installationId)
        }
    }

}
