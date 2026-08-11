pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula

Item {

    id: root

    property string label

    property list<var> initialItems
    property ListModel items: ListModel {}
    property int maxItems: 99

    readonly property int count: items.count

    function itemAt(i) {
        return itemRepeater.itemAt(i)
    }

    signal itemPopped(int index)

    Column {
        id: contentColumn
        width: parent.width

        spacing: 8

        Item {
            width: parent.width
            height: 24

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
                text: root.label
                color: Theme.colors.textDim
            }
        }

        Item {
            width: parent.width
            height: itemsColumn.height

            Column {
                id: itemsColumn
                width: parent.width - 8
                spacing: 8

                Repeater {
                    id: itemRepeater
                    model: root.items
                    delegate: Item {

                        required property string name
                        required property bool loggedIn



                    }
                }
            }
        }
    }


}
