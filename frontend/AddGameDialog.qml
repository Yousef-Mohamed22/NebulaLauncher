import QtQuick
import QtQuick.Layouts

NblDialog {

    id: root
    property var activeInstallation: null

    property bool newGame: root.activeInstallation == null && gameBox.currentIndex == 0

    NblComboBox {

        visible: root.activeInstallation == null

        id: gameBox
        label: "Game"
        width: parent.width
        highlightFirst: true

        textRole: "title"

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
        visible: !root.newGame
    }

    Column {
        visible: overrideBox.checked || root.newGame

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
        visible: root.newGame
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

    Component.onCompleted: {

        if(root.activeInstallation) {
            let details = root.activeInstallation

            let dateObj = new Date()
            dateObj.setTime(details.releaseDate)

            // TODO: Game box (e.g. gameBox.currentIndex = details.gameId)
            commandField.text = details.command
            overrideBox.checked = details.displayId != 0
            titleField.text = details.title
            descriptionField.text = details.description
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
        let dateObj = new Date()
        let rating = 0.0
        let steamId = 0
        let igdbId = 0

        if(root.newGame) {
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
        if(root.newGame || overrideBox.checked) {
            title = titleField.text
            description = descriptionField.text
            poster = posterPicker.imageSource
            icon = iconPicker.imageSource
            logo = logoPicker.imageSource
            banner = bannerPicker.imageSource
        }

        let gId = 0
        if(root.activeInstallation != null) {
            gId = root.activeInstallation.gameId
        } else {
            gId = gameBox.model.data(gameBox.model.index(gameBox.currentIndex, 0), 0); // 0 == IdRole
        }

        let params = {
            game: gId,
            type: commandTypeBox.currentIndex,
            command: commandField.text,
            displayOverride: overrideBox.checked,
            displayId: root.activeInstallation ? root.activeInstallation.displayId : 0,
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

        if(root.activeInstallation) {
            ApplicationContext.updateInstallation(root.activeInstallation.installationId, params)
        } else {
            ApplicationContext.addInstallation(params)
        }
    }
}
