import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: dateBox.height

    onValueChanged: {
        dateBox.setTime(value)
    }

    NblDateInput {
        id: dateBox
        width: parent.width
        label: root.field.name

        onDateEdited: {
            root.value = getTime()
        }
    }
}
