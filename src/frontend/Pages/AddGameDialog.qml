pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

NblDialog {

    id: root

    property int gameId: 0
    property string gameName: "+ New Game"
    property bool newGame: gameId == 0

    onGameIdChanged: {
        if(gameBox.accepting) return
        gameName = ApplicationContext.getGameDetails(gameId).title
    }

    NblScrollView {

        width: parent.width
        height: parent.height - 16
        y: 16

        clip: true

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        contentWidth: width - 48
        contentHeight: controls.height

        Column {

            id: controls
            x: 24
            width: parent.width
            spacing: 16

            NblComboBox {

                id: gameBox
                property bool accepting: false

                label: "Game"
                width: parent.width
                highlightFirst: true

                textRole: "title"
                valueRole: "id"

                displayText: root.gameName

                model: NblGameSearchModel {
                    includeNewGame: true
                    pageSize: 10
                }

                onAccepted: {
                    accepting = true
                    root.gameId = currentValue
                    root.gameName = currentText
                    accepting = false
                }
            }

            NblComboBox {
                id: commandTypeBox
                label: "Type"
                width: parent.width
                model: NblInstallationTypeModel { }
                textRole: "name"
                valueRole: "id"
            }

            NblFieldList { 
                id: fieldList
                typeId: commandTypeBox.currentValue
            }

            NblTextBox {
                visible: root.newGame
                id: gameTitleField
                label: "Game Title"
                width: parent.width
            }

            NblTextBox {
                visible: root.newGame
                id: gameDescriptionField
                label: "Game Description"
                width: parent.width
            }

            NblTextBox {
                id: titleField
                label: "Installation Title"
                placeholderText: "Optional"
                width: parent.width
            }

            NblTextBox {
                id: descriptionField
                label: "Installation Description"
                placeholderText: "Optional"
                width: parent.width
            }

            NblTagList {
                visible: root.newGame
                id: gameTags
                label: "Game Tags"
                width: parent.width
            }

            NblTagList {
                id: installationTags
                label: "Installation Tags"
                width: parent.width
            }

            RowLayout {

                width: parent.width
                height: 200

                NblImagePicker {
                    id: posterPicker
                    label: "Poster"
                    implicitHeight: parent.height
                    Layout.fillWidth: true
                    aspectRatio: 2.0 / 3.0
                }
                NblImagePicker {
                    id: iconPicker
                    label: "Icon"
                    implicitHeight: parent.height
                    Layout.fillWidth: true
                }
                NblImagePicker {
                    id: logoPicker
                    label: "Logo"
                    implicitHeight: parent.height
                    Layout.fillWidth: true
                    fillMode: Image.PreserveAspectFit
                }
            }

            NblImagePicker {
                id: bannerPicker
                label: "Banner"
                height: 200
                width: parent.width
                aspectRatio: 96.0 / 31.0
            }

            NblComboBox {
                id: logoPositionBox
                label: "Logo Position"
                width: parent.width
                model: root.newGame 
                    ? [ "Left", "Center", "Right", "Hidden" ]
                    : [ "Left", "Center", "Right", "Hidden", "Inherit" ]
                currentIndex: root.newGame ? 0 : 4
            }

            Column {
                visible: root.newGame
                spacing: 8
                width: parent.width
                NblDateInput {
                    id: releaseDateField
                    label: "Release Date"
                    width: parent.width
                }
                NblNumberInput {
                    id: ratingField
                    label: "Rating"
                    width: parent.width

                    allowDecimal: true
                    min: 0
                    max: 100

                    placeholderText: "100.0"
                }
            }
        }
    }

    onApply: {

        let rating = 0.0

        if(root.newGame) {
            if(ratingField.text.length != 0) {
                rating = ratingField.value
            }
        }

        let gId = root.gameId

        let command = fieldList.saveToJson()
        let params = {
            game: gId,
            type: commandTypeBox.currentValue,
            command: command,
            title: titleField.text,
            description: descriptionField.text,
            gameTitle: gameTitleField.text,
            gameDescription: gameDescriptionField.text,
            banner: bannerPicker.imageSource,
            poster: posterPicker.imageSource,
            icon: iconPicker.imageSource,
            logo: logoPicker.imageSource,
            logoPosition: logoPositionBox.currentIndex,
            releaseDate: releaseDateField.getTime(),
            rating: rating,
            addedInstallationTags: installationTags.getAddedTags(),
            removedInstallationTags: installationTags.getRemovedTags(),
            addedGameTags: gameTags.getAddedTags(),
            removedGameTags: gameTags.getRemovedTags()
        }

        ApplicationContext.addInstallation(params)
    }
}
