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

        controlsOffset: (root.field.fieldTemplate.type == 3 || root.field.fieldTemplate.name.length == 0) ? 0 : 16 * 0.9

        minItems: root.field.min
        maxItems: root.field.max

        createNew: function() { 
            let value = root.field.fieldTemplate.defaultValue
            root.value.push(value)
            return value
        }

        delegate: NblFieldInput {
            required property int index
            required property var modelData

            field: root.field.fieldTemplate
            width: parent.width

            Component.onCompleted: {
                load(modelData)
            }

            onModified: {
                if(modified) {
                    root.value[index] = save()
                }
            }
        }
    }
}
