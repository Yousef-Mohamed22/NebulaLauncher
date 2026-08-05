import QtQuick
import QtQuick.Layouts

Column {

    id: root

    spacing: 16

    property int gameId: 0

    property bool modified: false
    property alias icon: iconPicker.imageSource

    onGameIdChanged: {

        let details = ApplicationContext.getGameDetails(gameId)
        if(Object.keys(details).length == 0) return

        gameId = details.gameId

        gameTitleField.text = details.title
        gameTitleField.cursorPosition = 0 
        gameDescriptionField.text = details.description
        gameDescriptionField.cursorPosition = 0 

        gameTags.initialTags = ApplicationContext.getGameTags(details.gameId)

        let dataFolder = ApplicationContext.dataFolder

        if(details.poster) posterPicker.imageSource = "file://" + dataFolder + "/media/poster/" + details.poster
        if(details.icon) iconPicker.imageSource = "file://" + dataFolder + "/media/icon/" + details.icon
        if(details.logo) logoPicker.imageSource = "file://" + dataFolder + "/media/logo/" + details.logo
        if(details.banner) bannerPicker.imageSource = "file://" + dataFolder + "/media/banner/" + details.banner

        logoPositionBox.currentIndex = details.logoPosition

        releaseDateField.setTime(details.releaseDate)

        ratingField.value = details.rating

        modified = false
    }

    function encode() {

        return {
            id: root.gameId,
            title: gameTitleField.text,
            description: gameDescriptionField.text,
            poster: posterPicker.imageSource,
            icon: iconPicker.imageSource,
            logo: logoPicker.imageSource,
            banner: bannerPicker.imageSource,
            logoPosition: logoPositionBox.currentIndex,
            releaseDate: releaseDateField.getTime(),
            rating: ratingField.value,
            tags: gameTags.getAddedTags(),
            removedTags: gameTags.getRemovedTags()
        }
    }

    NblTextBox {
        id: gameTitleField
        label: "Title"
        width: root.width
        onTextEdited: { root.modified = true }
        tooltip: "The game's title. This will appear before the titles of installations of this game"
    }

    NblTextBox {
        id: gameDescriptionField
        label: "Description"
        width: root.width
        onTextEdited: { root.modified = true }
        tooltip: "The game's description"
    }

    NblTagList {
        id: gameTags
        label: "Tags"
        width: root.width
        onModified: { root.modified = true }
        tooltip: "Tags used to categorize this game. These tags will also apply to all installations of this game"
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
            tooltip: "The game's poster"
        }
        NblImagePicker {
            id: iconPicker
            label: "Icon"
            implicitHeight: parent.height
            Layout.fillWidth: true
            onImageSourceChanged: { root.modified = true }
            tooltip: "The game's icon"
        }
        NblImagePicker {
            id: logoPicker
            label: "Logo"
            implicitHeight: parent.height
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            onImageSourceChanged: { root.modified = true }
            tooltip: "The game's logo"
        }
    }

    NblImagePicker {
        id: bannerPicker
        label: "Banner"
        height: 200
        width: root.width
        aspectRatio: 96.0 / 31.0
        onImageSourceChanged: { root.modified = true }
        tooltip: "The game's banner"
    }

    NblComboBox {
        id: logoPositionBox
        label: "Logo Position"
        width: root.width
        model: [ "Left", "Center", "Right", "Hidden" ]
        onActivated: { root.modified = true }
        tooltip: "The position the logo will appear over the banner on an installation's detail page"
    }


    NblDateInput {
        id: releaseDateField
        label: "Release Date"
        width: root.width
        onDateEdited: { root.modified = true }
        tooltip: "The day the game was released"
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
        tooltip: "The game's rating from 0-100"
    }

}
