import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {

    id: root

    required property NblInstallationDetails details

    height: 72
    width: 212

    RectangularShadow {

        width: parent.width - 4
        height: parent.height - 4
        anchors.centerIn: parent
        radius: 8

        spread: 4
        color: "#FF000000"

    }

    Rectangle {

        id: background

        height: parent.height
        width: parent.width

        color: Theme.colors.accent
        radius: 8

        Row {
            spacing: 8
            anchors.centerIn: parent

            Label {
                text: ""
                font.family: Theme.nerdFont
                font.pixelSize: 28
                color: Theme.colors.text
            }
            Label {
                text: "LAUNCH"
                font.family: Theme.font
                font.pixelSize: 28
                font.bold: true
                color: Theme.colors.text
            }
        }
    }

    MouseArea {
        anchors.fill: background
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: {
            ApplicationContext.launchInstallation(root.details.installationId)
        }
    }
}

