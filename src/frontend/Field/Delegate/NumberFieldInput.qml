import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: numberBox.height

    onValueChanged: {
        numberBox.value = value
    }

    NblNumberInput {
        id: numberBox
        width: parent.width
        label: root.field.name
        min: root.field.min
        max: root.field.max
        allowDecimal: root.field.type == 2

        onValueModified: {
            root.value = value
        }
    }
}
