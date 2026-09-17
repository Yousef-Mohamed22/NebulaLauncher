import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: textBox.height

    onValueChanged: {
        textBox.text = value.raw
        textBox.clearSuggestions()
        field.suggest(value, function(suggestion) {
            textBox.pushSuggestion(suggestion)
        })
    }

    NblSuggestionsTextBox {
        id: textBox
        width: parent.width
        label: root.field.name

        onTextEdited: {
            if(root.value == null) {
                root.value = { 
                    raw: text,
                    resolved: ""
                }
            } else {
                root.value.raw = text
            }
        }
    }
}
