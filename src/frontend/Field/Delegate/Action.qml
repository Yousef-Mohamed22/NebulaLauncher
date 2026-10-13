import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: button.height

    NblButton {
        id: button
        width: root.width
        text: root.field.name
        onClicked: {
            root.field.action()
        }
    }
}
