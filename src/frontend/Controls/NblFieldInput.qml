pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root
    required property var field

    implicitHeight: 54

    property var value

    function getSourceComponent() {
        if(field.type == 0) return textBoxComponent
        if(field.type == 1 || field.type == 2) return numberBoxComponent
    }

    Loader {
        id: loader
        sourceComponent: root.getSourceComponent()
        active: true
    }

    Component {
        id: textBoxComponent
        NblTextBox {
            id: textBox
            width: root.width
            label: root.field.name

            Binding {
                target: textBox
                property: "text"
                value: root.value
            }

            onTextEdited: {
                root.value = text
            }
        }
    }

    Component {
        id: numberBoxComponent
        NblNumberInput {
            id: numberBox
            width: root.width
            label: root.field.name
            value: root.value

            Binding {
                target: numberBox
                property: "value"
                value: root.value
            }

            onValueModified: {
                root.value = value
            }
        }
    }
}
