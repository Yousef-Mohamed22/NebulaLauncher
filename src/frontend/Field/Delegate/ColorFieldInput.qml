import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: 48

    NblColorSwatch {
        id: swatch
        label: root.field?.name ?? ""

        onAccepted: {
            root.value = selectedColor
        }
    }
}
