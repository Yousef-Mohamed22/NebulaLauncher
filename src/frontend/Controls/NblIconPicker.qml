import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    implicitWidth: 70
    implicitHeight: 70

    property string icon: "󰮂"

    Rectangle {
        anchors.fill: parent
        radius: 8
        border.width: 2
        border.color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.border
        color: "transparent"
    }

    Rectangle {
        id: textBg
        width: iconText.width + 12
        height: iconText.height + 6
        color: Theme.colors.layer0
        x: 12
        y: -12
    }

    Label {
        id: iconText
        text: "Icon"
        anchors.centerIn: textBg
        font.family: Theme.font
        font.pixelSize: 16
        font.weight: 500
        color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
    }

    NblInlineIcon {
        anchors.fill: parent
        anchors.margins: 12
        icon: root.icon
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            popup.visible = true
        }
    }

    Popup {
        id: popup
        visible: false

        width: 100
        height: 100

        x: parent.height + 8

        background: Rectangle {
            radius: 8
            border.width: 2
            border.color: Theme.colors.border
            color: Theme.colors.layer1
        }

        contentItem: Item {



        }
    }
}
