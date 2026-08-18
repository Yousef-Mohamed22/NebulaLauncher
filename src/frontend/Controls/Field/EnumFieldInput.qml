import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: comboBox.height

    onValueChanged: {
        comboBox.currentIndex = value
    }

    NblComboBox {
        id: comboBox
        width: parent.width
        label: root.field.name
        model: root.field.enumModel
        textRole: "name"
        valueRole: "name"

        onActivated: {
            root.value = currentIndex
        }
    }
}
