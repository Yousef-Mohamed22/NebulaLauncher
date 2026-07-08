pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    property string label
    property bool allowReorder: false

    property list<var> initialItems
    property ListModel items: ListModel {}

    readonly property int count: items.count
    readonly property int controlsWidth: allowReorder ? 64 : 48

    required property var createNew
    required property Component delegate

    function itemAt(i) {
        return itemRepeater.itemAt(i)
    }

    signal itemPopped(int index)
    signal itemMoved(int oldIndex, int currentIndex)

    implicitHeight: contentColumn.height

    onInitialItemsChanged: {

        items.clear()
        for(let i = 0 ; i < initialItems.length ; i++) {
            items.append({ modelData: initialItems[i] })
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
                width: parent.width - root.controlsWidth - 8
                spacing: 8

                Repeater {
                    id: itemRepeater
                    model: root.items
                    delegate: root.delegate

                    function getItemHeight(i, def) {

                        if(i >= count) return def
                        return itemAt(i).height
                    }
                }
            }

            Column {
                width: root.controlsWidth
                x: parent.width - width
                spacing: 8
                Repeater {
                    model: root.count

                    Item {

                        id: itemControls
                        required property int modelData
                        readonly property int index: modelData

                        width: parent.width
                        height: itemRepeater.getItemHeight(index, 96)

                        RowLayout {

                            spacing: 0

                            width: parent.width
                            height: Math.min(48, parent.height)

                            ColumnLayout {

                                visible: root.allowReorder
                                spacing: 0

                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Item {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Rectangle { 
                                        id: upBg
                                        anchors.fill: parent;
                                        color: upArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                                        radius: 8
                                    }
                                    Label {
                                        anchors.centerIn: upBg
                                        text: ""
                                        font.family: Theme.font
                                        font.pixelSize: 20
                                        font.weight: 600
                                        color: itemControls.index == 0 ? Theme.colors.textDim : Theme.colors.text
                                    }

                                    MouseArea {
                                        id: upArea
                                        visible: itemControls.index > 0
                                        anchors.fill: upBg
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            let index = itemControls.index
                                            if(index == 0) return

                                            root.items.move(index, index - 1, 1)
                                            root.itemMoved(index, index - 1)
                                        }
                                    }
                                }
                                Item {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Rectangle {
                                        id: downBg
                                        anchors.fill: parent
                                        color: downArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                                        radius: 8
                                    }

                                    Label {
                                        anchors.centerIn: downBg
                                        text: ""
                                        font.family: Theme.font
                                        font.pixelSize: 20
                                        font.weight: 600
                                        color: (itemControls.index + 1) == root.count ? Theme.colors.textDim : Theme.colors.text
                                    }

                                    MouseArea {
                                        id: downArea
                                        visible: (itemControls.index + 1) < root.count
                                        anchors.fill: downBg
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            let index = itemControls.index
                                            if((index + 1) == root.count) return

                                            root.items.move(index, index + 1, 1)
                                            root.itemMoved(index, index + 1)
                                        }
                                    }
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Rectangle {
                                    id: removeBg
                                    anchors.fill: parent
                                    color: removeArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                                    radius: 8
                                }

                                Label {
                                    anchors.centerIn: removeBg
                                    text: ""
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
                                        root.itemPopped(itemControls.index)
                                        root.items.remove(itemControls.index, 1)
                                    }
                                }
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
                    root.items.append({ modelData: root.createNew(root.items.count) })
                }
            }
        }
    }

}
