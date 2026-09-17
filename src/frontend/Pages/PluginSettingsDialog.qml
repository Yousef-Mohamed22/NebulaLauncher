import QtQuick
import QtQuick.Controls
import Nebula

NblDialog {

    id: root

    property string pluginName
    required property NblFieldList fields
    required property var values

    Component.onCompleted: {
        fields.load(values)
    }

    onValuesChanged: {
        fields.load(values)
    }

    function save() {
        return fields.save()
    }

    NblScrollView {
        width: parent.width + 16
        height: parent.height

        contentWidth: parent.width
        contentHeight: contentColumn.height

        Column {
            width: parent.width
            id: contentColumn 

            spacing: 16

            Label {
                text: root.pluginName + " Settings"
                font.family: Theme.font
                font.weight: 500
                font.pixelSize: 16
            }

            NblFieldInputRepeater {
                width: parent.width
                id: fields
                fields: root.fields
            }
        }
    } 
}
