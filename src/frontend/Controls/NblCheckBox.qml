import QtQuick
import QtQuick.Controls

CheckBox {

    id: root

    font.family: Theme.font
    font.pixelSize: 16

    implicitHeight: indicator.height
    leftPadding: 0

    indicator: Rectangle {
        implicitWidth: 32
        implicitHeight: 32        
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: 8

        color: Theme.colors.layer0
        border.width: 2
        border.color: root.checked ? Theme.colors.accent : Theme.colors.border

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            color: Theme.colors.accent
            radius: 4
            visible: root.checked
        }
    }

    contentItem: Text {
        text: root.text
        font: root.font
        color: root.checked ? Theme.colors.text : Theme.colors.textDim
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
    }

}
