pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {

    id: root
    property int count: 0

    property string label
    property list<var> initialItems

    required property var createNew
    required property Component delegate

    function itemAt(i) {
        return itemsColumn.children[i]
    }

    signal itemPopped(int index)

    implicitHeight: contentColumn.height

    onInitialItemsChanged: {

        for(let i = 0 ; i < count ; i++) {
            itemPopped(i)
            itemAt(i).destroy()
            count--
        }
        for(let i = 0 ; i < initialItems.length ; i++) {
            let item = initialItems[i]
            delegate.createObject(itemsColumn, {
                "modelData": item
            })
            count++
        }
    }

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
                width: parent.width - 48 - 8
                spacing: 8
            }

            Column {
                width: 48
                x: parent.width - width
                spacing: 8
                Repeater {
                    model: root.count
                    Item {

                        required property int modelData
                        readonly property int index: modelData

                        width: parent.width
                        height: root.itemAt(index) == null ? 48 : root.itemAt(index).height

                        Rectangle {
                            id: removeBg
                            width: parent.width
                            height: parent.width

                            color: removeArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                            radius: 8
                        }

                        Label {
                            anchors.centerIn: removeBg
                            text: "-"
                            font.family: Theme.font
                            font.pixelSize: 24
                            font.weight: 600
                            color: Theme.colors.text
                        }

                        MouseArea {
                            id: removeArea
                            anchors.fill: removeBg
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.itemPopped(parent.index)
                                root.itemAt(parent.index).destroy()
                                root.count--
                            }
                        }
                    }
                }
            }
        }

        Item {
            width: parent.width
            height: 48

            Rectangle {
                anchors.fill: parent
                color: addArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                radius: 8
            }

            Label {
                anchors.fill: parent
                anchors.leftMargin: 16
                verticalAlignment: Qt.AlignVCenter
                font.family: Theme.font
                font.pixelSize: 16
                font.weight: 600
                text: "+ New"
                color: Theme.colors.text
            }

            MouseArea {
                id: addArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    let item = root.createNew(root.count)
                    root.delegate.createObject(itemsColumn, {
                        "modelData": item
                    })
                    root.count++
                }
            }
        }
    }

}
