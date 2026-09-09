pragma ComponentBehavior: Bound
import QtQuick

Repeater {

    id: root

    required property NblFieldList fields
    property bool modified: false

    property int itemWidth: width

    model: root.fields ? root.fields.count : 0
    delegate: NblFieldInput {
        required property var index
        field: root.fields.itemAt(index)
        width: root.itemWidth

        onModifiedChanged: {
            if(modified && !root.modified) {
                root.modified = true
            }
        }
    }

    function load(encodedFields) {
        for (const [key, value] of Object.entries(encodedFields)) {
            for(let i = 0 ; i < root.count ; i++) {
                let item = root.itemAt(i) as NblFieldInput
                if(item.field.key == key) {
                    item.load(value)
                }
            }
        }
        modified = false
    }

    function save() {
        let fields = {}
        for(let i = 0 ; i < root.count ; i++) {
            let field = root.itemAt(i) as NblFieldInput
            fields[field.field.key] = field.save()
        }
        modified = false
        return fields
    }
}
