import QtQuick
import QtQuick.Controls

import Nebula

Item {

    id: root
    property string label
    property int indent: 12

    implicitHeight: 16

    Rectangle {
        width: parent.width
        height: 2
        y: (parent.height / 2) - 1
        color: Theme.colors.border
    }
    Rectangle {
        id: labelBg
        width: labelLabel.width + 12
        height: parent.height
        visible: root.label.length > 0
        x: root.indent
        color: Theme.colors.layer0
    }
    Label {
        id: labelLabel
        anchors.centerIn: labelBg
        font.family: Theme.font
        font.pixelSize: 16
        font.weight: 500
        text: root.label
        color: Theme.colors.textDim
    }
}
