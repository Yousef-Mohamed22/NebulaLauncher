import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: contentColumn.height

    onValueChanged: {
        fieldInput.load(value)
    }

    Column {

        id: contentColumn
        spacing: 8
        width: parent.width

        Item {

            visible: root.field.name.length > 0

            width: parent.width
            height: 30

            Rectangle {
                width: parent.width
                height: 2
                y: (parent.height / 2) - 1
                color: Theme.colors.border
            }

            Rectangle {
                id: labelBg
                width: labelLabel.width + 12
                height: parent.height
                x: 12
                color: Theme.colors.layer0
            }

            Label {
                id: labelLabel
                anchors.centerIn: labelBg
                font.family: Theme.font
                font.pixelSize: 16
                font.weight: 500
                text: root.field.name
                color: Theme.colors.textDim
            }
        }
        GridLayout {

            id: layout
            width: parent.width
            columns: root.field.horizontal ? fieldInput.count : 1
            rowSpacing: 8
            columnSpacing: 8

            NblFieldInputRepeater {
                id: fieldInput
                fields: root.field.structFields
                itemWidth: root.field.horizontal ? ((layout.width + layout.columnSpacing) / count) - layout.columnSpacing : layout.width

                onModifiedChanged: {
                    if(modified) {
                        root.value = save()
                    }
                }
            }
        }
    }
}
