pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

SettingsTab {

    id: root

    onRefreshed: {
        searchModel.refresh()
        flickable.contentY = 0
    }

    onApplied: {

        let modified = encodeModifiedGames()
        for(let i = 0 ; i < modified.length ; i++) {
            let title = modified[i].title
            let res = ApplicationContext.updateGame(modified[i])
            if(res != 0) {
                ApplicationContext.pushNotification({
                    type: "error",
                    title: "Failed to Update a Game",
                    message: "An error occurred while updating the game \"" + title + "\""
                })
                console.log("An error occurred while updating a game!", res)
            }
        }

        let removed = getRemovedGameIds()
        for(let i = 0 ; i < removed.length ; i++) {
            let res = ApplicationContext.deleteGame(removed[i])
            if(res != 0) {
                ApplicationContext.pushNotification({
                    type: "error",
                    title: "Failed to Delete a Game",
                    message: "An error occurred while deleting a game. This is usually because there still exist installations of that game."
                })
                console.log("An error occurred while deleting a game!", res)
            }
        }

        refresh()
    }

    function encodeModifiedGames() {

        let output = []

        for(let i = 0 ; i < gameRepeater.count ; i++) {
            let gameElement = gameRepeater.itemAt(i)
            if(gameElement.modified && !gameElement.removed) {
                output.push(gameElement.encode())
            }
        }

        return output
    }

    function getRemovedGameIds() {
        let output = []
        for(let i = 0 ; i < gameRepeater.count ; i++) {
            let gameElement = gameRepeater.itemAt(i)
            if(gameElement.removed) {
                output.push(gameElement.getId())
            }
        }
        return output
    }

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
            property NblDropdown currentlyOpenDropdown: null

            Repeater {

                id: gameRepeater

                model: NblGameSearchModel {
                    id: searchModel
                    searchQuery: filterBox.text
                    pageSize: 10
                }

                delegate: Item {

                    width: root.width
                    height: dropdown.height

                    id: gameElement

                    required property var modelData
                    property bool modified: gameSettings.modified
                    property bool removed: false

                    function getId() {
                        return modelData.id
                    }

                    function encode() {
                        return gameSettings.encode()
                    }

                    NblDropdown {

                        id: dropdown

                        width: parent.width - collapsedHeight
                        label: gameElement.modelData.title

                        contentHeight: gameSettings.height
                        enabled: !gameElement.removed

                        property bool loadedTags: false
                        onCollapsedChanged: {
                            if(!collapsed) {

                                if(gameColumn.currentlyOpenDropdown && gameColumn.currentlyOpenDropdown != this) {
                                    gameColumn.currentlyOpenDropdown.collapsed = true
                                }
                                gameColumn.currentlyOpenDropdown = this

                                if(!loadedTags) {
                                    loadedTags = true
                                    gameSettings.gameId = gameElement.modelData.id
                                }
                            }
                        }

                        NblGameSettings {
                            id: gameSettings
                            width: dropdown.contentWidth
                        }
                    }


                    Item {
                        width: dropdown.collapsedHeight
                        height: width

                        x: gameElement.width - width

                        Rectangle {
                            anchors.fill: parent
                            color: removeArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                            radius: 8
                        }

                        Label {
                            text: gameElement.removed ? "" : ""
                            font.family: Theme.nerdFont
                            font.pixelSize: 20
                            anchors.centerIn: parent
                            color: Theme.colors.text
                        }

                        MouseArea {
                            id: removeArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                gameElement.removed = !gameElement.removed
                            }
                        }
                    }

                    Rectangle {
                        width: dropdown.width
                        height: 8
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.colors.layer0
                        visible: gameElement.removed
                    }

                    Rectangle {
                        width: dropdown.width
                        height: 2
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.colors.textDim
                        visible: gameElement.removed
                    }
                }
            }
        }
    }
}

