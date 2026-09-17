import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: input.height

    NblFieldInput {
        id: input
        width: root.width
        field: root.field.fieldTemplate
        onModified: {
            root.field.fieldAction(save())
        }
    }
}
