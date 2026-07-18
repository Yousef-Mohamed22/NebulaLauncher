import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    property bool collapsed: true
    property real contentHeight: 48
    property real collapsedHeight: 48
    property string label: ""

    readonly property real contentWidth: width - collapsedHeight

    default property alias content: contentContainer.children
    readonly property bool effectivelyCollapsed: collapsed || !enabled

    implicitHeight: effectivelyCollapsed ? collapsedHeight : contentHeight

    onEnabledChanged: {
        if(!enabled) {
            collapsed = true
        }
    }


    Item {

        width: root.effectivelyCollapsed ? parent.width : root.collapsedHeight
        height: root.collapsedHeight

        Rectangle {
            anchors.fill: parent
            visible: root.effectivelyCollapsed
            color: root.enabled && collapseMouseArea.containsMouse ? Theme.colors.layer1 : "transparent"
            radius: 8
        }

        Label {

            width: root.collapsedHeight
            height: root.collapsedHeight

            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter

            text: root.effectivelyCollapsed ? "" : ""
            font.family: Theme.font
            font.pixelSize: 20
            color: Theme.colors.textDim

        }

        MouseArea {
            id: collapseMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.collapsed = !root.collapsed
            }
        }

        Label {
            visible: root.effectivelyCollapsed

            height: root.collapsedHeight
            x: root.collapsedHeight

            verticalAlignment: Qt.AlignVCenter

            text: root.label
            font.family: Theme.font
            font.pixelSize: 16
            color: Theme.colors.text
        }
    }

    Item {

        visible: !root.effectivelyCollapsed
        width: parent.width
        height: root.contentHeight

        Rectangle {
            width: 2
            height: parent.height - root.collapsedHeight
            y: root.collapsedHeight
            x: (root.collapsedHeight / 2) - 1
            color: Theme.colors.border
        }

        Item {
            id: contentContainer

            width: root.contentWidth
            height: root.contentHeight
            x: root.collapsedHeight
        }

    }

}
