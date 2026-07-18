pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    NblTextBox {
        id: filterBox
        width: parent.width
        label: "Filter"
    }

    Flickable {

        width: parent.width
        height: parent.height - 64
        y: 64

        id: flickable

        contentWidth: width
        contentHeight: gameColumn.height
        flickableDirection: Flickable.VerticalFlick
        clip: true

        ScrollBar.horizontal: null
        ScrollBar.vertical: ScrollBar {
            parent: flickable
            anchors.top: flickable.top
            anchors.bottom: flickable.bottom
            anchors.right: flickable.right
            active: flickable.movingVertically
        }

        onAtYEndChanged: {
            if(atYEnd) {
                searchModel.loadNextPage()
            }
        }

        Column {

            id: gameColumn

            Repeater {

                model: NblGameSearchModel {
                    id: searchModel
                    pageSize: 10
                    searchQuery: filterBox.text
                }

                delegate: Item {

                    width: root.width
                    height: 64 //height: dropdown.height

                    id: gameElement

                    required property var modelData
                    property bool modified: false
                    property bool removed: false

                    Label {
                        text: parent.modelData.title
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.colors.text
                        font.family: Theme.font
                        font.pixelSize: 16
                    }

                    // Item {
                    //     width: dropdown.collapsedHeight
                    //     height: width
                    //
                    //     x: gameElement.width - width
                    //
                    //     Rectangle {
                    //         anchors.fill: parent
                    //         color: removeArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                    //         radius: 8
                    //     }
                    //
                    //     Label {
                    //         text: gameElement.removed ? "" : ""
                    //         font.family: Theme.nerdFont
                    //         font.pixelSize: 20
                    //         anchors.centerIn: parent
                    //         color: Theme.colors.text
                    //     }
                    //
                    //     MouseArea {
                    //         id: removeArea
                    //         anchors.fill: parent
                    //         hoverEnabled: true
                    //         cursorShape: Qt.PointingHandCursor
                    //         onClicked: {
                    //             gameElement.removed = !gameElement.removed
                    //         }
                    //     }
                    // }
                    //
                    // Rectangle {
                    //     width: dropdown.width
                    //     height: 8
                    //     anchors.verticalCenter: parent.verticalCenter
                    //     color: Theme.colors.layer0
                    //     visible: gameElement.removed
                    // }
                    //
                    // Rectangle {
                    //     width: dropdown.width
                    //     height: 2
                    //     anchors.verticalCenter: parent.verticalCenter
                    //     color: Theme.colors.textDim
                    //     visible: gameElement.removed
                    // }
                    
                }
            }
        }
    }
}

