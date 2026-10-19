// Copyright 2026 jwall64
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula

NblDialog {

    id: root

    showApplyButton: false

    property int gameId: 0
    property string gameName: "+ New Game"
    property bool newGame: gameId == 0

    signal complete(bool success, bool gameAdded, var gameId, var installationId)

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

                model: GameSearchModel {
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
                model: InstallationTypeModel { 
                    id: typeModel
                }
                textRole: "name"
                valueRole: "id"
            }

            FieldInputRepeater { 
                id: fieldList
                width: parent.width
                fields: typeModel.fields(commandTypeBox.currentIndex)
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

            TagList {
                visible: root.newGame
                id: gameTags
                label: "Game Tags"
                width: parent.width
            }

            TagList {
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
                    tooltip: "Poster override for this installation"
                }
                NblImagePicker {
                    id: iconPicker
                    label: "Icon"
                    implicitHeight: parent.height
                    Layout.fillWidth: true
                    tooltip: "Icon override for this installation"
                }
                NblImagePicker {
                    id: logoPicker
                    label: "Logo"
                    implicitHeight: parent.height
                    Layout.fillWidth: true
                    fillMode: Image.PreserveAspectFit
                    tooltip: "Logo override for this installation"
                }
            }

            NblImagePicker {
                id: bannerPicker
                label: "Banner"
                height: 200
                width: parent.width
                aspectRatio: 96.0 / 31.0
                tooltip: "Banner override for this installation"
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

    function load(meta) {

        gameTitleField.text = meta.title
        gameDescriptionField.text = meta.description
        
        if(meta.posterUrl.length > 0) posterPicker.imageSource = meta.posterUrl
        if(meta.iconUrl.length > 0) iconPicker.imageSource = meta.iconUrl
        if(meta.bannerUrl.length > 0) bannerPicker.imageSource = meta.bannerUrl
        if(meta.logoUrl.length > 0) logoPicker.imageSource = meta.logoUrl

        logoPositionBox.currentIndex = meta.logoPosition
        ratingField.value = meta.rating
        releaseDateField.setTime(meta.releaseDate)

        gameTags.initialTags = meta.tags
    }


    onApply: {

        let rating = 0.0

        if(root.newGame) {
            if(ratingField.text.length != 0) {
                rating = ratingField.value
            }
        }

        let gId = root.gameId

        let params = {
            game: gId,
            type: commandTypeBox.currentValue,
            fields: fieldList.save(),
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

        let res = ApplicationContext.addInstallation(params)
        root.complete(res.success, res.addedGame, res.gameId, res.installationId)
    }
}
