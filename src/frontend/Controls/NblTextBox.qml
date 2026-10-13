import QtQuick
import QtQuick.Controls.Basic

import Nebula

TextField {

    id: root
    property string label
    property string tooltip
    property int tooltipDelay: 1000

    readonly property int backgroundOffset: (label == null || label.length == 0) ? 0 : root.font.pixelSize * 0.9

    color: enabled ? Theme.colors.text : Theme.colors.textDim
    placeholderTextColor: Theme.colors.border

    font.pixelSize: 16
    font.family: Theme.font

    implicitHeight: 48 + backgroundOffset

    topPadding: 8 + backgroundOffset
    leftPadding: 16
    rightPadding: 16

    background: Rectangle {
        color: Theme.colors.layer0
        height: parent.height - root.backgroundOffset
        y: root.backgroundOffset
        radius: 8
        border.width: 2
        border.color: root.cursorVisible ? Theme.colors.accent : Theme.colors.border
    }

    Item {

        visible: root.label && root.label.length > 0

        width: 0
        height: 0
        x: 16

        Rectangle { 
            width: labelText.width + 12
            height: labelText.height - 8
            anchors.centerIn: labelText
            color: Theme.colors.layer0 

            NblToolTip {
                delay: root.tooltipDelay
                text: root.tooltip
                visible: hover.containsMouse
            }

            MouseArea {
                id: hover
                anchors.fill: parent
                hoverEnabled: root.tooltip.length > 0
            }
        }
        Label { 
            id: labelText
            text: root.label
            font.family: Theme.font
            font.weight: 500
            color: root.enabled 
                ? root.cursorVisible 
                    ? Theme.colors.accent 
                    : Theme.colors.textDim
                : Theme.colors.border
        }
    }
}
