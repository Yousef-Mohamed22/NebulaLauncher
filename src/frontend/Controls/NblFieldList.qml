pragma ComponentBehavior: Bound
import QtQuick

Repeater {
    id: root

    required property string typeId

    signal modified

    model: NblFieldModel {
        id: fieldModel
        typeId: root.typeId
    }

    NblFieldInput {
        required property var modelData
        required property var index

        width: parent.width
        field: modelData
        fieldIndex: index
        typeId: root.typeId

        onModified: {
            root.modified()
        }
    }

    function loadFromJson(commandJson) {

        let fields = JSON.parse(commandJson)
        for (const [key, value] of Object.entries(fields)) {
            for(let i = 0 ; i < count ; i++) {
                let item = itemAt(i) as NblFieldInput
                if(item.field.key == key) {
                    item.setValue(value)
                }
            }
        }
    }

    function saveToJson() {
        let fields = {}
        for(let i = 0 ; i < count ; i++) {
            let field = itemAt(i) as NblFieldInput
            fields[field.field.key] = field.value
        }
        return JSON.stringify(fields)

    }

}
