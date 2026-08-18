import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: textBox.height

    onValueChanged: {
        textBox.text = value
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
            root.value = text
        }
    }
}
