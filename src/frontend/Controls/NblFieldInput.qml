import QtQuick

import Nebula
import Nebula.Controls.Field

Item {

    id: root

    property NblFieldRef field
    property bool modified

    implicitHeight: loader.item == null ? 0 : (loader.item as Item).height

    function save() {
        if(loader.item == null) return null
        modified = false
        let out = (loader.item as FieldInputDelegate).value
        return out
    }

    function load(value) {
        if(loader.item == null) {
            console.log("Not loaded!")
            return null
        }
        (loader.item as FieldInputDelegate).value = value
        modified = false
    }


    Loader {
        id: loader
        width: parent.width
    }

    Connections {
        target: loader.item
        function onValueChanged() {
            root.modified = true
        }
    }

    onFieldChanged: {

        if(field == null) return

        let source = null
        if(field.type == 0) {
            source = "Field/StringFieldInput.qml"
        } else if(field.type == 1 || field.type == 2) {
            source = "Field/NumberFieldInput.qml"
        } else if(field.type == 3) {
            source = "Field/BoolFieldInput.qml"
        } else if(field.type == 4) {
            source = "Field/EnumFieldInput.qml"
        } else if(field.type == 5) {
            source = "Field/DateFieldInput.qml"
        } else if(field.type == 6) {
            source = "Field/AccountFieldInput.qml"
        } else if(field.type == 7) {
            source = "Field/ListFieldInput.qml"
        } else if(field.type == 8) {
            source = "Field/StructFieldInput.qml"
        }

        if(source == null) {
            console.warn("Unknown field type: ", field.type)
            return
        }
        loader.setSource(source, {
            "field": field
        })
        loader.active = true
    }
}

