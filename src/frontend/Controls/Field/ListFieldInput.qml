pragma ComponentBehavior: Bound
import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: listInput.height

    onValueChanged: {
        listInput.initialItems = value
    }

    NblListInput {

        id: listInput
        width: parent.width
        label: root.field.name

        minItems: root.field.min
        maxItems: root.field.max

        createNew: { 
            return root.field.listTemplate.defaultValue
        }

        delegate: NblFieldInput {
            required property int index
            required property var modelData

            field: root.field.listTemplate
            width: parent.width

            Component.onCompleted: {
                load(modelData)
            }

            onModifiedChanged: {
                if(modified) {
                    root.value[index] = save()
                }
            }
        }
    }
}
