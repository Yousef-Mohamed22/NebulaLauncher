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
        echoMode: root.field.password ? TextInput.Password : TextInput.Normal

        onTextEdited: {
            root.value = { 
                raw: text,
                resolved: text
            }
        }
    }
}
