import QtQuick
import QtQuick.Controls

ToolTip {

    id: root

    padding: 8
    background: Rectangle {
        color: Theme.colors.layer0
        border.width: 2
        border.color: Theme.colors.border
        radius: 8
    }
    contentItem: Text {
        text: root.text
        font: root.font
        color: Theme.colors.text
    }
}

