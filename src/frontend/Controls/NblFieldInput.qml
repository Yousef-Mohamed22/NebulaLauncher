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

            let textBox = loader.item as NblTextBox
            textBox.text = root.value

        } else if(root.field.type == 1 || root.field.type == 2) {

            let numberBox = loader.item as NblNumberInput
            numberBox.value = root.value

        } else if(root.field.type == 3) {

            let checkBox = loader.item as NblCheckBox
            checkBox.checked = root.value

        } else if(root.field.type == 4) {

            let comboBox = loader.item as NblComboBox
            comboBox.currentIndex = enumModel.indexOf(root.value)
        }
    }

    Loader {
        id: loader
        width: root.width
        active: false

        onLoaded: {

            if(root.field.type == 0) {

                let textBox = loader.item as NblTextBox
                textBox.text = root.value
                textBox.textEdited.connect(() => {
                    root.value = textBox.text
                })

            } else if(root.field.type == 1 || root.field.type == 2) {

                let numberBox = loader.item as NblNumberInput
                numberBox.value = root.value
                numberBox.valueModified.connect(() => {
                    root.value = numberBox.value
                })

            } else if(root.field.type == 3) {

                let checkBox = loader.item as NblCheckBox
                checkBox.checked = root.value
                checkBox.toggled.connect(() => {
                    root.value = checkBox.checked
                })
            } else if(root.field.type == 4) {

                let comboBox = loader.item as NblComboBox
                comboBox.currentValue = root.value
                comboBox.activated.connect(() => {
                    root.value = comboBox.currentValue.toString()
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
