pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula

Item {
    id: root

    property string text: ""
    property list<var> badges: []
    property color color: Theme.colors.text

    property alias font: titleText.font

    implicitHeight: titleText.height

    Row {
        id: row
        width: parent.width
        Text {
            id: titleText
            width: Math.max(
                0,
                Math.min(
                    implicitWidth,
                    root.width - (16 + row.spacing) * root.badges.length
                )
            )

            maximumLineCount: 1
            text: root.text

            verticalAlignment: Qt.AlignVCenter

            font.family: Theme.font
            font.pixelSize: 16
            font.weight: 600

            color: root.color
            elide: Text.ElideRight
        }
        Repeater {
            model: root.badges
            delegate: Item {
                id: badgeItem
                required property var modelData
                height: root.height
                width: height
                Label {
                    anchors.fill: parent
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    text: badgeItem.modelData.icon
                    color: root.color
                }
                NblToolTip {
                    text: badgeItem.modelData.text
                    delay: 500
                    visible: progressionHover.containsMouse
                }
                MouseArea {
                    id: progressionHover
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }
        }
    }
}

