pragma ComponentBehavior: Bound
import QtQuick

Repeater {

    id: root

    required property NblFieldList fields
    property bool wasModified: false

    signal modified

    property int itemWidth: width

    model: root.fields ? root.fields.visibleCount : 0
    delegate: NblFieldInput {
        required property var index
        field: root.fields ? root.fields.visibleItemAt(index) : null
        width: root.itemWidth

        onModified: {
            root.wasModified = true
            root.modified()
        }
    }

    function load(encodedFields) {
        if(encodedFields) {
            for (const [key, value] of Object.entries(encodedFields)) {
                for(let i = 0 ; i < root.count ; i++) {
                    let item = root.itemAt(i) as NblFieldInput
                    if(item.field.key == key) {
                        item.load(value)
                    }
                }
            }
        } else {
            for(let i = 0 ; i < root.count ; i++) {
                let item = root.itemAt(i) as NblFieldInput
                item.load(null)
            }
        }
        wasModified = false
    }

    function save() {
        let fields = {}
        for(let i = 0 ; i < root.count ; i++) {
            let field = root.itemAt(i) as NblFieldInput
            fields[field.field.key] = field.save()
        }
        wasModified = false
        return fields
    }
}
