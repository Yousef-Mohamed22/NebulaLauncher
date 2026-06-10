import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property MainWindow mainWindow

    width: parent.width
    height: parent.height

    Rectangle {
        anchors.fill: parent
        color: "#AA000000"
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
    }

    Item {
        width: Math.min(parent.width - 16, 800)
        height: Math.min(parent.height - 16, 600)

        anchors.centerIn: parent

        Rectangle {
            anchors.fill: parent
            color: Theme.colors.layer0
            radius: 16
        }

        ColumnLayout {

            anchors.fill: parent
            anchors.margins: 16
            spacing: 8

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 16
                id: flickable

                contentWidth: width - 48
                contentHeight: content.height
                flickableDirection: Flickable.VerticalFlick
                clip: true

                Column {

                    id: content
                    width: flickable.contentWidth
                    x: 24

                    spacing: 8

                    NblComboBox {
                        id: gameBox
                        label: "Game"
                        width: parent.width
                        highlightFirst: true

                        model: [ "New Game" ]
                    }

                    NblComboBox {
                        id: commandTypeBox
                        label: "Type"
                        width: parent.width
                        model: [ "Command" ]
                    }

                    NblTextBox {
                        id: commandField
                        label: "Command"
                        width: parent.width
                    }

                    NblCheckBox {
                        id: overrideBox
                        text: "Override display assets for this installation"
                        visible: gameBox.currentIndex != 0
                    }

                    Column {
                        visible: overrideBox.checked || gameBox.currentIndex == 0

                        width: parent.width
                        spacing: 8

                        NblTextBox {
                            id: titleField
                            label: "Title"
                            width: parent.width
                        }
                        NblTextBox {
                            id: descriptionField
                            label: "Description"
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
                            model: [ "Left", "Center", "Right", "Hidden" ]
                        }

                    }

                    Column {
                        visible: gameBox.currentIndex == 0
                        spacing: 8
                        width: parent.width
                        NblTextBox {
                            id: releaseDateField
                            label: "Release Date"
                            width: parent.width
                            placeholderText: "YYYY/MM/DD"
                            text: Qt.formatDate(new Date(), "yyyy/MM/dd")
                            validator: RegularExpressionValidator { 
                                regularExpression: /^\d{4}\/(0[1-9]|1[0-2])\/(0[1-9]|[12]\d|3[01])$/ 
                            }
                        }
                        NblTextBox {
                            id: ratingField
                            label: "Rating"
                            width: parent.width

                            placeholderText: "100.0"
                            text: "0.0"
                            inputMethodHints: Qt.ImhFormattedNumbersOnly

                            validator: DoubleValidator {
                                bottom: 0.0
                                top: 100.0
                                decimals: 1                
                                notation: DoubleValidator.StandardNotation
                            }
                        }
                        NblTextBox {
                            id: steamIdField
                            label: "Steam ID"
                            width: parent.width

                            placeholderText: "620"
                            text: "0"
                            inputMethodHints: Qt.ImhDigitsOnly

                            validator: IntValidator {
                                bottom: 0
                            }
                        }
                        NblTextBox {
                            id: igdbIdField
                            label: "IGDB ID"
                            width: parent.width

                            placeholderText: "7346"
                            text: "0"
                            inputMethodHints: Qt.ImhDigitsOnly

                            validator: IntValidator {
                                bottom: 0
                            }
                        }
                    }

                }

                ScrollBar.horizontal: null
                ScrollBar.vertical: ScrollBar {
                    parent: flickable
                    anchors.top: flickable.top
                    anchors.bottom: flickable.bottom
                    anchors.right: flickable.right
                    active: flickable.movingVertically
                }

            }

            Item {

                Layout.fillWidth: true
                Layout.preferredHeight: 40

                Row {

                    anchors.fill: parent

                    layoutDirection: Qt.RightToLeft
                    spacing: 8

                    Item {

                        id: addBtn
                        width: 128
                        height: parent.height

                        Rectangle {
                            anchors.fill: parent
                            color: Theme.colors.accent
                            radius: 8
                        }

                        Label {
                            anchors.centerIn: parent
                            font.pixelSize: 16
                            font.weight: 500
                            color: Theme.colors.text
                            text: "Add"
                        }

                        MouseArea {
                            id: addMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.complete()
                            }
                        }

                    }
                    
                    Item {

                        id: cancelBtn
                        width: 128
                        height: parent.height

                        Rectangle {
                            anchors.fill: parent
                            color: cancelMouseArea.containsMouse ? Theme.colors.border : Theme.colors.layer1
                            radius: 8
                        }

                        Label {
                            anchors.centerIn: parent
                            font.pixelSize: 16
                            font.weight: 500
                            color: Theme.colors.text
                            text: "Cancel"
                        }

                        MouseArea {
                            id: cancelMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.close()
                            }
                        }

                    }

                }

            }
        }
    }

    function complete() {

        // Validate
        let dateObj = new Date()
        let rating = 0.0
        let steamId = 0
        let igdbId = 0

        if(gameBox.currentIndex > 0) {
            if(releaseDateField.text.length != 0) {
                dateObj = Date.fromLocaleDateString(Qt.locale(), releaseDateField.text, "yyyy/MM/dd")
                if (isNaN(dateObj.getTime())) {
                    console.log("Invalid date entered!")
                    return; // TODO: Report errors in UI
                } 
            }

            if(ratingField.text.length != 0) {
                rating = Math.max(0.0, Math.min(100.0, parseFloat(ratingField.text)))
            }

            if(steamIdField.text.length != 0) {
                steamId = Math.max(0, parseInt(steamIdField.text))
            }

            if(igdbIdField.text.length != 0) {
                igdbId = Math.max(0, parseInt(igdbIdField.text))
            }
        }

        let title = null
        let description = null
        let poster = null
        let icon = null
        let logo = null
        let banner = null
        if(gameBox.currentIndex == 0 || overrideBox.checked) {
            title = titleField.text
            description = descriptionField.text
            poster = posterPicker.imageSource
            icon = iconPicker.imageSource
            logo = logoPicker.imageSource
            banner = bannerPicker.imageSource
        }

        let params = {
            game: gameBox.currentIndex,
            type: commandTypeBox.currentIndex,
            command: commandField.text,
            displayOverride: overrideBox.checked,
            title: title,
            description: description,
            banner: banner,
            poster: poster,
            icon: icon,
            logo: logo,
            logoPosition: logoPositionBox.currentIndex,
            releaseDate: dateObj.getTime(),
            rating: rating,
            steamId: steamId,
            igdbId: igdbId
        }

        ApplicationContext.addInstallation(params)
        ApplicationContext.reload()
        close()
    }

    function close() {

        root.mainWindow.finishAddGame()
    }
}
