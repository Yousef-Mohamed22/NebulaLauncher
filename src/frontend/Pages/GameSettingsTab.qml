pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

SettingsTab {

    id: root

    property bool addingNew

    onRefreshed: {
        searchModel.refresh()
        flickable.contentY = 0
    }

    onApplied: {

        let modified = encodeModifiedGames()
        for(let i = 0 ; i < modified.length ; i++) {
            let title = modified[i].title
            let res = ApplicationContext.updateGame(modified[i])
            if(res < 0) {
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
            if(res < 0) {
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


    RowLayout {

        width: parent.width
        height: 54
        
        NblTextBox {
            id: filterBox
            Layout.fillWidth: true
            label: "Filter"

            enabled: !root.addingNew
        }
        
        NblButton {

            id: newBtn

            icon: root.addingNew ? "" : ""
            text: root.addingNew ? "Cancel" : "New"

            Layout.preferredWidth: 128
            Layout.preferredHeight: 48
            Layout.topMargin: 6

            inactiveColor: Theme.colors.layer1
            centered: true

            onClicked: {

                if(root.addingNew) {

                    root.addingNew = false

                } else {

                    if(root.modified) {
                        ApplicationContext.pushNotification({
                            type: "error",
                            title: "Cannot Add Game",
                            message: "Please apply your changes first."
                        })
                        return
                    }

                    root.addingNew = true
                }
            }
        }
    }

    NblScrollView {

        width: parent.width
        height: parent.height - 64
        y: 64

        id: flickable

        contentWidth: width
        contentHeight: root.addingNew ? newGameColumn.height : gameColumn.height
        clip: true

        onAtYEndChanged: {
            if(atYEnd && !root.addingNew) {
                searchModel.loadNextPage()
            }
        }

        Column {
            id: newGameColumn
            visible: root.addingNew

            spacing: 16

            NblGameSettings {
                id: newGameSettings
                width: root.width
            }

            Item {

                width: 128
                height: 48

                Rectangle {
                    anchors.fill: parent
                    color: Theme.colors.accent
                    radius: 8
                }

                Label {
                    anchors.centerIn: parent
                    font.pixelSize: 16
                    font.weight: 500
                    color: Theme.colors.textOnAccent
                    text: "Add"
                }

                MouseArea {
                    id: addMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {

                        let rc = ApplicationContext.addGame(newGameSettings.encode())
                        if(rc != 0) {
                            ApplicationContext.pushNotification({
                                type: "error",
                                title: "Could Not Add Game",
                                message: "An error occurred."
                            })
                            console.log("An error occurred while adding a game", rc)
                        }

                        root.addingNew = false
                        root.refresh()
                    }
                }
            }
        }

        Column {

            id: gameColumn
            visible: !root.addingNew

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
                        icon: gameSettings.gameId == 0 
                            ? ApplicationContext.mediaUrl(gameElement.modelData.icon, "icon")
                            : gameSettings.icon
                        colorizeIcon: false
                        iconRadius: 8
                        bold: gameSettings.modified

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

                            onModifiedChanged: {
                                if(modified) {
                                    root.modified = true
                                }
                            }
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

