pragma ComponentBehavior: Bound

import QtQuick

Repeater {
    id: root

    required property NblFieldList fields
    signal modified

    model: fields.count
    delegate: NblFieldInput {
        required property var index
        field: root.fields.itemAt(index)
        width: parent.width

        onModified: {
            root.modified()
        }
    }

    function loadFromJson(commandJson) {

        let encodedFields = JSON.parse(commandJson)
        for (const [key, value] of Object.entries(encodedFields)) {
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
