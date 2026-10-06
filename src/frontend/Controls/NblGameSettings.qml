import QtQuick
import QtQuick.Controls
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

        if(details.poster.length > 0) posterPicker.imageSource = ApplicationContext.mediaUrl(details.poster, "poster")
        if(details.icon.length > 0) iconPicker.imageSource = ApplicationContext.mediaUrl(details.icon, "icon")
        if(details.logo.length > 0) logoPicker.imageSource = ApplicationContext.mediaUrl(details.logo, "logo")
        if(details.banner.length > 0) bannerPicker.imageSource = ApplicationContext.mediaUrl(details.banner, "banner")

        logoPositionBox.currentIndex = details.logoPosition

        releaseDateField.setTime(details.releaseDate)

        ratingField.value = details.rating

        let ids = ApplicationContext.gameExternalIds(gameId)
        let externalIds = []
        for(let platformName in ids) {
            externalIds.push({ platform: platformName, id: ids[platformName] })
        }
        platformList.initialItems = externalIds

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
            removedTags: gameTags.getRemovedTags(),
            externalIds: platformList.encode(),
            hiddenSets: shownSets.encode(),
            achievementProviders: achievementsColumn.saveAll()
        }
    }

    NblButton {
        leftPadding: 24
        rightPadding: 24
        icon: ""
        text: "Lookup Metadata"
        activeColor: Theme.colors.accent
        inactiveColor: Theme.colors.accent
        onClicked: {
            let dialog = DialogManager.create("Pages/MetadataSearchDialog.qml", {
                targetWidth: 700,
                targetHeight: 700,
                title: gameTitleField.text,
                releaseDate: releaseDateField.getTime(),
                platformIds: platformList.encode()
            }) as MetadataSearchDialog
            dialog.apply.connect(function() {

                let meta = dialog.collect()
                if("poster" in meta) { posterPicker.imageSource = meta.poster }
                if("icon" in meta) { iconPicker.imageSource = meta.icon }
                if("banner" in meta) { bannerPicker.imageSource = meta.banner }
                if("logo" in meta) { logoPicker.imageSource = meta.logo }

                if("tags" in meta) {
                    for(let i = 0 ; i < meta.tags.length ; i++) {

                        let tag = meta.tags[i]
                        if(tag.tagId == 0) {
                            tag.tagId = ApplicationContext.addTag(tag.name, tag.color)
                            if(tag.tagId == 0) {
                                ApplicationContext.pushNotification({
                                    type: "error",
                                    title: "Failed to add tag",
                                    message: "An error occurred while adding a tag!"
                                })
                                return
                            }
                        }
                        gameTags.addTag(tag.tagId, tag.name, tag.color)
                    }
                }

                if("title" in meta) { gameTitleField.text = meta.title }
                if("description" in meta) { gameDescriptionField.text = meta.description }
                if("releaseDate" in meta) { releaseDateField.setTime(meta.releaseDate) }
                if("rating" in meta) { ratingField.value = meta.rating }

            })
            dialog.open()
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


    Column {
        id: achievementsColumn
        width: parent.width
        visible: achievementsRepeater.count > 0

        NblSectionHeader {
            width: parent.width
            label: "Achievement Providers"
        }

        function saveAll() {
            let out = {}
            for(let i = 0 ; i < achievementsRepeater.count ; i++) {
                let item = achievementsRepeater.itemAt(i)
                out[item.modelData.id] = {
                    fields: item.save(),
                    status: item.providerStatus,
                    originalStatus: item.modelData.status
                }
            }
            return out
        }

        Repeater {
            id: achievementsRepeater
            model: NblAchievementStateModel {
                id: achievementsModel
                gameId: root.gameId
            }
            delegate: Column {

                id: providerColumn
                width: achievementsColumn.width - 12
                x: 12
                spacing: 8

                required property var modelData
                required property int index

                property int providerStatus: modelData.status
                property bool added: providerStatus == 1 || providerStatus == 2

                function save() { return added ? fieldList.save() : {} }

                Component.onCompleted: {
                    fieldList.load(modelData.fieldValues)
                }

                RowLayout {
                    width: parent.width
                    height: 40
                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: providerColumn.modelData.name
                        verticalAlignment: Qt.AlignVCenter
                    }
                    NblButton {
                        Layout.preferredWidth: 128
                        Layout.fillHeight: true
                        text: providerColumn.added ? "Remove" : "Add"
                        icon: providerColumn.added ? '' : ""
                        centered: true
                        activeColor: providerColumn.added ? Theme.colors.border : Theme.colors.accent
                        inactiveColor: providerColumn.added ? Theme.colors.layer1 : Theme.colors.accent
                        onClicked: {
                            if(providerColumn.added) {
                                providerColumn.providerStatus = 4
                            } else {
                                providerColumn.providerStatus = 2
                            }
                            console.log(providerColumn.providerStatus)
                        }
                    }
                }

                NblFieldInputRepeater {
                    id: fieldList
                    width: parent.width
                    fields: providerColumn.added ? achievementsModel.fields(parent.index) : null
                }
            }
        }
    }


    Column {
        id: shownSets
        width: parent.width
        visible: setsRepeater.count > 0
        spacing: 8

        function encode() {
            let out = []
            for(let i = 0 ; i < setsRepeater.count ; i++) {
                let item = setsRepeater.itemAt(i) as NblCheckBox
                out.push({ 
                    id: item.modelData.id,
                    hidden: !item.checked
                })
            }
            return out
        }

        NblSectionHeader {
            width: parent.width
            label: "Show Achievement Sets"
        }
        
        Repeater {
            id: setsRepeater
            model: NblAchievementSetsModel {
                gameId: root.gameId
                includeHidden: true
            }
            delegate: NblCheckBox {
                required property var modelData
                x: 12
                width: shownSets.width - 12
                elide: Text.ElideRight
                text: modelData.name
                checked: !modelData.hidden
            }
        }
    }

    NblListInput {
        id: platformList
        width: parent.width
        label: "External IDs"
        controlsOffset: gameTitleField.backgroundOffset

        function encode() {
            let out = []
            for(let i = 0 ; i < count ; i++) {
                out.push(itemAt(i).modelData)
            }
            return out;
        }

        createNew: function() {
            return { platform: "", id: "" }
        }
        delegate: RowLayout {
            required property var modelData
            width: platformList.itemWidth

            NblTextBox {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 200
                label: "Platform Name"
                text: parent.modelData.platform
                onTextEdited: {
                    parent.modelData.platform = text
                }
            }

            NblTextBox {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 200
                label: "ID"
                text: parent.modelData.id
                onTextEdited: {
                    parent.modelData.id = text
                }
            }
        }
    }
}
