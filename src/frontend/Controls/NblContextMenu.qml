import QtQuick

import QtQuick.Controls

Popup { 
    id: contextPopup
    visible: false

    implicitWidth: 200

    x: parent.width + 16
    y: parent.height - height

    margins: 0
    padding: 2

    background: Rectangle {
        color: Theme.colors.layer1
        border.width: 2
        border.color: Theme.colors.border
        radius: 8
    }
}
