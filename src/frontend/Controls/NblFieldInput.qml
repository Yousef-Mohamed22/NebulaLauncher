pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root

    required property NblFieldRef field
    property var value

    signal modified

    implicitHeight: field.type == 3 ? 32 : 54

    function setValue(newValue) {

        console.log(typeof field)

        root.value = newValue
        if(root.field.type == 0) {

            let textBox = loader.item as NblSuggestionsTextBox
            textBox.text = newValue

        } else if(root.field.type == 1 || root.field.type == 2) {

            let numberBox = loader.item as NblNumberInput
            numberBox.value = newValue

        } else if(root.field.type == 3) {

            let checkBox = loader.item as NblCheckBox
            checkBox.checked = newValue

        } else if(root.field.type == 4) {

            let comboBox = loader.item as NblComboBox
            comboBox.currentIndex = enumModel.indexOf(newValue)
        
        } else if(root.field.type == 5) {

            let dateBox = loader.item as NblDateInput
            dateBox.setTime(newValue)

        }
    }

    Loader {
        id: loader
        width: root.width
        active: false

        onLoaded: {
            root.value = root.field.defaultValue
            if(root.field.type == 0) {

                let textBox = loader.item as NblSuggestionsTextBox
                textBox.text = root.value
                textBox.textEdited.connect(() => {
                    root.value = textBox.text
                    root.modified()

                    textBox.clearSuggestions()
                    root.field.suggest(textBox.text, function(suggestion) {
                        textBox.pushSuggestion(suggestion)
                    })
                })
                root.field.suggest(textBox.text, function(suggestion) {
                    textBox.pushSuggestion(suggestion)
                })

            } else if(root.field.type == 1 || root.field.type == 2) {

                let numberBox = loader.item as NblNumberInput
                numberBox.value = root.value
                numberBox.valueModified.connect(() => {
                    root.value = numberBox.value
                    root.modified()
                })

            } else if(root.field.type == 3) {

                let checkBox = loader.item as NblCheckBox
                checkBox.checked = root.value
                checkBox.toggled.connect(() => {
                    root.value = checkBox.checked
                    root.modified()
                })
            } else if(root.field.type == 4) {

                let comboBox = loader.item as NblComboBox
                comboBox.currentValue = root.value
                comboBox.activated.connect(() => {
                    root.value = comboBox.currentValue.toString()
                    root.modified()
                })
            } else if(root.field.type == 5) {

                let dateBox = loader.item as NblDateInput
                dateBox.setTime(root.value)
                dateBox.dateEdited.connect(() => {
                    root.value = dateBox.getTime()
                    root.modified()
                })
            }
        }
    }
    

    property NblEnumFieldModel enumModel: field.enumModel()

    Component.onCompleted: {

        root.value = root.field.defaultValue

        if(root.field.type == 0) {

            loader.setSource("NblSuggestionsTextBox.qml", {
                "width": root.width,
                "label": root.field.name
            })

        } else if(root.field.type == 1 || root.field.type == 2) {

            loader.setSource("NblNumberInput.qml", {
                "width": root.width,
                "label": root.field.name,
                "min": root.field.min,
                "max": root.field.max,
                "allowDecimal": root.field.type == 2
            })

        } else if(root.field.type == 3) {

            loader.setSource("NblCheckBox.qml", {
                "width": root.width,
                "text": root.field.name
            })

        } else if(root.field.type == 4) {

            loader.setSource("NblComboBox.qml", {
                "width": root.width,
                "label": root.field.name,
                "model": root.enumModel,
                "textRole": "name",
                "valueRole": "name"
            })

        } else if(root.field.type == 5) {

            loader.setSource("NblDateInput.qml", {
                "width": root.width,
                "label": root.field.name,
            })
        }

        loader.active = true
    }
}
