import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: input.height

    FieldInput {
        id: input
        width: root.width
        field: root.field.fieldTemplate
        onModified: {
            root.field.fieldAction(save())
        }
    }
}
