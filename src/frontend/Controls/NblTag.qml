pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Control {

    id: root

    required property string text
    required property color color

    property int tooltipDelay: 1000
    
    padding: 4
    implicitWidth: textDisplay.implicitWidth + leftPadding + rightPadding
    implicitHeight: textDisplay.implicitHeight + topPadding + bottomPadding

    property int textSize: 14

    signal clicked

    background: Rectangle {
        anchors.fill: parent
        color: root.color
        radius: root.height / 2
    }

    contentItem: Item {

        Text {

            id: textDisplay
            text: root.text

            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            elide: Qt.ElideRight

            width: root.availableWidth
            height: root.availableHeight

            font.pixelSize: root.textSize
            font.family: Theme.font
            font.weight: 600
            color: Theme.colors.readableOn(root.color)
        }

        NblToolTip {
            text: root.text
            delay: root.tooltipDelay
            visible: hover.containsMouse
        }

        MouseArea {
            id: hover
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.clicked()
            }
        }
    }
}
