pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic

import Nebula

BusyIndicator {
    id: root

    property color color: Theme.colors.text
    property string icon: "󱥸"

    contentItem: Item {

        implicitWidth: 16
        implicitHeight: 16

        Text {
            id: textItem
            text: root.icon
            color: root.color
            anchors.fill: parent
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        RotationAnimator {
            target: textItem
            running: root.visible && root.running
            from: 0
            to: 360
            loops: Animation.Infinite
            duration: 1250
        }

    }
}
