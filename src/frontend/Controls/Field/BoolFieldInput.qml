import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: checkBox.height

    onValueChanged: {
        checkBox.checked = value
    }

    NblCheckBox {
        id: checkBox
        width: parent.width
        text: root.field.name

        onToggled: {
            root.value = checked
        }
    }
}
