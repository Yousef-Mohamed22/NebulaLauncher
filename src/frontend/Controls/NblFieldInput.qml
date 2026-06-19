pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root
    required property var field
    required property int fieldIndex
    required property string typeId
    
    property var value

    implicitHeight: field.type == 3 ? 32 : 54

    property var valueProperty: "text"
    property var valueSignal: "onTextEdited"

    function setValue(newValue) {
        value = newValue
        if(root.field.type == 0) {
            loader.item.text = root.value
        } else if(root.field.type == 1 || root.field.type == 2) {
            loader.item.value = root.value
        } else if(root.field.type == 3) {
            loader.item.checked = root.value
        } else if(root.field.type == 4) {
            loader.item.currentIndex = enumModel.indexOf(root.value)
        }
    }

    Loader {
        id: loader
        width: root.width
        active: false

        onLoaded: {

            if(root.field.type == 0) {
                loader.item.text = root.value
                loader.item.textEdited.connect(() => {
                    root.value = loader.item.text
                })
            } else if(root.field.type == 1 || root.field.type == 2) {
                loader.item.value = root.value
                loader.item.valueModified.connect(() => {
                    root.value = loader.item.value
                })
            } else if(root.field.type == 3) {
                loader.item.checked = root.value
                loader.item.toggled.connect(() => {
                    root.value = loader.item.checked
                })
            } else if(root.field.type == 4) {
                loader.item.currentValue = root.value
                loader.item.activated.connect(() => {
                    root.value = loader.item.currentValue
                })
            }
        }
    }
    

    property NblEnumFieldModel enumModel: NblEnumFieldModel {
        typeId: root.typeId
        fieldIndex: root.fieldIndex

        onRowsInserted: {
            if(root.field.type == 4 && loader.status == Loader.Ready) {
                loader.item.currentValue = root.value
            }
        }
    }

    Component.onCompleted: {

        root.value = root.field.defaultValue

        if(root.field.type == 0) {

            root.valueProperty = "text"
            loader.setSource("NblTextBox.qml", {
                "width": root.width,
                "label": root.field.name
            })

        } else if(root.field.type == 1 || root.field.type == 2) {

            root.valueProperty = "value"
            loader.setSource("NblNumberInput.qml", {
                "width": root.width,
                "label": root.field.name,
                "min": root.field.min,
                "max": root.field.max,
                "allowDecimal": root.field.type == 2
            })

        } else if(root.field.type == 3) {
            root.valueProperty = "checked"
            loader.setSource("NblCheckBox.qml", {
                "width": root.width,
                "text": root.field.name
            })

        } else if(root.field.type == 4) {

            root.valueProperty = "currentValue"
            loader.setSource("NblComboBox.qml", {
                "width": root.width,
                "label": root.field.name,
                "model": root.enumModel,
                "textRole": "name",
                "valueRole": "name"
            })
        }

        loader.active = true
    }
}
