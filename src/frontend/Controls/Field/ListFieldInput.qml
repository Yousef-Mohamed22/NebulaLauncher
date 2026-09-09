pragma ComponentBehavior: Bound
import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: listInput.height
    value: []

    onValueChanged: {
        listInput.initialItems = value
    }

    NblListInput {

        id: listInput
        width: parent.width
        label: root.field.name

        controlsOffset: root.field.listTemplate.type == 3 ? 0 : 16 * 0.9

        minItems: root.field.min
        maxItems: root.field.max

        createNew: function() { 
            let value = root.field.listTemplate.defaultValue
            root.value.push(value)
            return value
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
