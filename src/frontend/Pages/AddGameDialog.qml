pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

NblDialog {

    id: root
    property var activeInstallation: null
    property bool newGame: root.activeInstallation == null && gameBox.currentIndex == 0

    showApplyButton: activeInstallation != null

    ScrollView {

        width: parent.width
        height: parent.height - 16
        y: 16

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        contentWidth: width - 48
        contentHeight: controls.height

        Flickable {

            anchors.fill: parent
            id: flickable
            clip: true

            Column {

                id: controls
                x: 24
                width: parent.width
                spacing: 16

                NblComboBox {

                    visible: root.activeInstallation == null

                    id: gameBox
                    label: "Game"
                    width: parent.width
                    highlightFirst: true

                    textRole: "title"
                    valueRole: "id"

                    model: NblGameModel {
                        includeNewGame: true
                        pageSize: 10
                    }
                }

                NblTextBox {

                    visible: root.activeInstallation != null

                    width: parent.width
                    enabled: false
                    label: "Game"
                    text: visible ? root.activeInstallation.gameTitle : ""
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
                    id: gameTitleField
                    label: "Game Title"
                    width: parent.width
                }

                NblTextBox {
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
                    id: gameTags
                    label: "Game Tags"
                    width: parent.width
                }

                NblTagList {
                    id: installationTags
                    label: "Installation Tags"
                    width: parent.width
                }

                NblCheckBox {
                    id: overrideBox
                    text: "Override display assets for this installation"
                    visible: !root.newGame
                }

                Column {
                    visible: overrideBox.checked || root.newGame

                    width: parent.width
                    spacing: 8

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
                        model: [ "Left", "Center", "Right", "Hidden" ]
                    }

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
                    NblNumberInput {
                        id: steamIdField
                        label: "Steam ID"
                        width: parent.width

                        min: 0
                        max: 2147483647

                        placeholderText: "620"
                    }
                    NblNumberInput {
                        id: igdbIdField
                        label: "IGDB ID"
                        width: parent.width

                        min: 0
                        max: 2147483647

                        placeholderText: "7346"
                    }
                }
            }
        }
    }

    Component.onCompleted: {

        if(root.activeInstallation) {
            let details = root.activeInstallation

            let dateObj = new Date()
            dateObj.setTime(details.releaseDate)

            commandTypeBox.currentValue = details.type
            fieldList.loadFromJson(details.command)
            
            titleField.text = details.title
            titleField.cursorPosition = 0
            descriptionField.text = details.description
            descriptionField.cursorPosition = 0

            gameTitleField.text = details.gameTitle
            gameTitleField.cursorPosition = 0
            gameDescriptionField.text = details.gameDescription
            gameDescriptionField.cursorPosition = 0

            gameTags.initialTags = ApplicationContext.getGameTags(root.activeInstallation.gameId);
            installationTags.initialTags = ApplicationContext.getInstallationTags(root.activeInstallation.installationId);

            overrideBox.checked = details.displayId != 0
            posterPicker.imageSource = details.posterPath()
            bannerPicker.imageSource = details.bannerPath()
            iconPicker.imageSource = details.iconPath()
            logoPicker.imageSource = details.logoPath()
            logoPositionBox.currentIndex = details.logoPosition
            releaseDateField.text = dateObj.toLocaleDateString(Qt.locale(), "yyyy/MM/dd")
            ratingField.text = details.rating.toFixed(1)
            steamIdField.text = details.steamId
            igdbIdField.text = details.igdbId
        }
    }

    onApply: {

        // Validate
        let rating = 0.0
        let steamId = 0
        let igdbId = 0

        if(root.newGame) {
            if(ratingField.text.length != 0) {
                rating = ratingField.value
            }

            if(steamIdField.text.length != 0) {
                steamId = steamIdField.value
            }

            if(igdbIdField.text.length != 0) {
                igdbId = igdbIdField.value
            }
        }

        let poster = null
        let icon = null
        let logo = null
        let banner = null
        if(root.newGame || overrideBox.checked) {
            poster = posterPicker.imageSource
            icon = iconPicker.imageSource
            logo = logoPicker.imageSource
            banner = bannerPicker.imageSource
        }

        let gId = 0
        if(root.activeInstallation != null) {
            gId = root.activeInstallation.gameId
        } else {
            gId = gameBox.currentValue;
        }

        let command = fieldList.saveToJson()
        let params = {
            game: gId,
            type: commandTypeBox.currentValue,
            command: command,
            displayOverride: overrideBox.checked,
            displayId: root.activeInstallation ? root.activeInstallation.displayId : 0,
            title: titleField.text,
            description: descriptionField.text,
            gameTitle: gameTitleField.text,
            gameDescription: gameDescriptionField.text,
            banner: banner,
            poster: poster,
            icon: icon,
            logo: logo,
            logoPosition: logoPositionBox.currentIndex,
            releaseDate: releaseDateField.getTime(),
            rating: rating,
            steamId: steamId,
            igdbId: igdbId,
            addedInstallationTags: installationTags.getAddedTags(),
            removedInstallationTags: installationTags.getRemovedTags(),
            addedGameTags: gameTags.getAddedTags(),
            removedGameTags: gameTags.getRemovedTags()
        }

        if(root.activeInstallation) {
            ApplicationContext.updateInstallation(root.activeInstallation.installationId, params)
        } else {
            ApplicationContext.addInstallation(params)
        }
    }
}
