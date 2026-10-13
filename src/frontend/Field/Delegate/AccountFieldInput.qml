import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: accountPicker.height

    onValueChanged: {
        accountPicker.selectedId = value
    }

    AccountPicker {
        id: accountPicker
        width: parent.width
        label: root.field.name
        accountProvider: root.field.accountProvider
        showDefaultOption: root.field.showDefaultOption
        onSelectedIdChanged: {
            root.value = selectedId
        }
    }
}
