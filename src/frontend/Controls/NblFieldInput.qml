import QtQuick

import Nebula
import Nebula.Controls.Field

Item {

    id: root

    property NblFieldRef field
    property bool wasModified

    signal modified

    implicitHeight: loader.item == null ? 0 : (loader.item as Item).height

    function save() {
        if(loader.item == null) return null
        wasModified = false
        return (loader.item as FieldInputDelegate).value
    }

    function load(value) {
        if(loader.item == null) {
            console.log("Not loaded!")
            return null
        }
        (loader.item as FieldInputDelegate).value = value
        wasModified = false
    }


    Loader {
        id: loader
        width: parent.width
    }

    Connections {
        target: loader.item
        function onValueChanged() {
            root.wasModified = true
            root.modified()
        }
    }

    onFieldChanged: {

        if(field == null) return

        let source = null
        switch(field.type) {
            case 0:
                source = "Field/StringFieldInput.qml"
                break
            case 1:
            case 2:
                source = "Field/NumberFieldInput.qml"
                break
            case 3:
                source = "Field/BoolFieldInput.qml"
                break
            case 4:
                source = "Field/EnumFieldInput.qml"
                break
            case 5:
                source = "Field/DateFieldInput.qml"
                break
            case 6:
                source = "Field/AccountFieldInput.qml"
                break
            case 7:
                source = "Field/ListFieldInput.qml"
                break
            case 8:
                source = "Field/StructFieldInput.qml"
                break
            case 9:
                source = "Field/Action.qml"
                break
            case 10:
                source = "Field/ActionFieldInput.qml"
                break
            case 11:
                source = "Field/CustomFieldInput.qml"
                break
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

