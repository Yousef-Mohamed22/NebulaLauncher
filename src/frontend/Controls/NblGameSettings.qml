import QtQuick
import QtQuick.Layouts

Column {

    id: root

    spacing: 16

    property int gameId: 0
    property int displayId: 0

    property bool modified: false

    onGameIdChanged: {

        let details = ApplicationContext.getGameDetails(gameId)
        if(Object.keys(details).length == 0) return

        gameId = details.id
        displayId = details.displayId

        gameTitleField.text = details.title
        gameDescriptionField.text = details.description

        gameTags.initialTags = details.tags

        let dataFolder = ApplicationContext.dataFolder

        if(details.poster) posterPicker.imageSource = "file://" + dataFolder + "/media/poster/" + details.poster
        if(details.icon) iconPicker.imageSource = "file://" + dataFolder + "/media/icon/" + details.icon
        if(details.logo) logoPicker.imageSource = "file://" + dataFolder + "/media/logo/" + details.logo
        if(details.banner) bannerPicker.imageSource = "file://" + dataFolder + "/media/banner/" + details.banner

        logoPositionBox.currentIndex = details.logoPosition

        releaseDateField.setTime(details.releaseDate)

        ratingField.value = details.rating
        steamIdField.value = details.steamId
        igdbIdField.value = details.igdbId

        modified = false
    }

    function encode() {

        return {
            id: root.gameId,
            displayId: root.displayId,
            title: gameTitleField.text,
            description: gameDescriptionField.text,
            poster: posterPicker.imageSource,
            icon: iconPicker.imageSource,
            logo: logoPicker.imageSource,
            banner: bannerPicker.imageSource,
            logoPosition: logoPositionBox.currentIndex,
            releaseDate: releaseDateField.getTime(),
            rating: ratingField.value,
            steamId: steamIdField.value,
            igdbId: igdbIdField.value,
            tags: gameTags.getAddedTags(),
            removedTags: gameTags.getRemovedTags()
        }
    }

    NblTextBox {
        id: gameTitleField
        label: "Title"
        width: root.width
        onTextEdited: { root.modified = true }
    }

    NblTextBox {
        id: gameDescriptionField
        label: "Description"
        width: root.width
        onTextEdited: { root.modified = true }
    }

    NblTagList {
        id: gameTags
        label: "Tags"
        width: root.width
        onModified: { root.modified = true }
    }
    
    RowLayout {

        width: root.width
        height: 200

        NblImagePicker {
            id: posterPicker
            label: "Poster"
            implicitHeight: parent.height
            Layout.fillWidth: true
            aspectRatio: 2.0 / 3.0
            onImageSourceChanged: { root.modified = true }
        }
        NblImagePicker {
            id: iconPicker
            label: "Icon"
            implicitHeight: parent.height
            Layout.fillWidth: true
            onImageSourceChanged: { root.modified = true }
        }
        NblImagePicker {
            id: logoPicker
            label: "Logo"
            implicitHeight: parent.height
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            onImageSourceChanged: { root.modified = true }
        }
    }

    NblImagePicker {
        id: bannerPicker
        label: "Banner"
        height: 200
        width: root.width
        aspectRatio: 96.0 / 31.0
        onImageSourceChanged: { root.modified = true }
    }

    NblComboBox {
        id: logoPositionBox
        label: "Logo Position"
        width: root.width
        model: [ "Left", "Center", "Right", "Hidden" ]
        onAccepted: { root.modified = true }
    }


    NblDateInput {
        id: releaseDateField
        label: "Release Date"
        width: root.width
        onDateEdited: { root.modified = true }
    }
    NblNumberInput {
        id: ratingField
        label: "Rating"
        width: root.width

        allowDecimal: true
        min: 0
        max: 100

        placeholderText: "100.0"
        onValueModified: { root.modified = true }
    }
    NblNumberInput {
        id: steamIdField
        label: "Steam ID"
        width: root.width

        min: 0
        max: 2147483647

        placeholderText: "620"
        onValueModified: { root.modified = true }
    }
    NblNumberInput {
        id: igdbIdField
        label: "IGDB ID"
        width: root.width

        min: 0
        max: 2147483647

        placeholderText: "7346"
        onValueModified: { root.modified = true }
    }

}
