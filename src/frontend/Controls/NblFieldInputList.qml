pragma ComponentBehavior: Bound
import QtQuick

Column {

    id: root
    spacing: 16

    required property NblFieldList fields
    property bool modified: false

    Repeater {
        id: repeater
        model: root.fields.count
        delegate: NblFieldInput {
            required property var index
            field: root.fields.itemAt(index)
            width: root.width

            onModifiedChanged: {
                if(modified && !root.modified) {
                    root.modified = true
                }
            }
        }
    }

    function load(encodedFields) {
        for (const [key, value] of Object.entries(encodedFields)) {
            for(let i = 0 ; i < repeater.count ; i++) {
                let item = repeater.itemAt(i) as NblFieldInput
                if(item.field.key == key) {
                    item.load(value)
                }
            }
        }
        modified = false
    }

    function save() {
        let fields = {}
        for(let i = 0 ; i < repeater.count ; i++) {
            let field = repeater.itemAt(i) as NblFieldInput
            fields[field.field.key] = field.save()
        }
        modified = false
        return fields
    }
}
