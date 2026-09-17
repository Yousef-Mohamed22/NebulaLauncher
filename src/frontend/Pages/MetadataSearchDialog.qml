pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula


NblDialog {

    id: root

    property int totalResults

    property string title
    property var releaseDate

    onReleaseDateChanged: {
        releaseDateBox.setTime(releaseDate)
    }

    property var platformIds

    function collect() {
        let output = {}
        if(posterModel.selectedIndex != -1) {
            output["poster"] = posterModel.get(posterModel.selectedIndex).value
        }
        if(iconModel.selectedIndex != -1) {
            output["icon"] = iconModel.get(iconModel.selectedIndex).value
        }
        if(bannerModel.selectedIndex != -1) {
            output["banner"] = bannerModel.get(bannerModel.selectedIndex).value
        }
        if(logoModel.selectedIndex != -1) {
            output["logo"] = logoModel.get(logoModel.selectedIndex).value
        }

        if(addedTagModel.count > 0) {
            let tags = []
            for(let i = 0 ; i < addedTagModel.count ; i++) {
                tags.push({
                    tagId: addedTagModel.get(i).tagId,
                    name: addedTagModel.get(i).text,
                    color: addedTagModel.get(i).color
                })
            }
            output["tags"] = tags
        }
        
        if(titleModel.selectedIndex != -1) {
            output["title"] = titleModel.get(titleModel.selectedIndex).value
        }
        if(descriptionModel.selectedIndex != -1) {
            output["description"] = descriptionModel.get(descriptionModel.selectedIndex).value
        }
        if(releaseModel.selectedIndex != -1) {
            output["releaseDate"] = releaseModel.get(releaseModel.selectedIndex).value
        }
        if(ratingModel.selectedIndex != -1) {
            output["rating"] = ratingModel.get(ratingModel.selectedIndex).value
        }
        return output
    }

    NblScrollView {

        width: parent.width + 10
        height: parent.height
        clip: true

        contentHeight: contentColumn.height

        MetadataSearchHelper {
            id: searchHelper

            onReceivedResult: (field, value) => {
                root.totalResults++
                if(field == 0) { titleModel.append({value: value}) }
                else if(field == 1) { descriptionModel.append({value: value}) }
                else if(field == 2) { releaseModel.append({value: value}) }
                else if(field == 3) { ratingModel.append({value: value}) }
                else if(field == 4) { tagModel.append({value: value}) }
                else if(field == 5) { posterModel.append({value: value}) }
                else if(field == 6) { bannerModel.append({value: value}) }
                else if(field == 7) { iconModel.append({value: value}) }
                else if(field == 8) { logoModel.append({value: value}) }
            }
        }

        Column {

            id: contentColumn
            width: parent.width - 10
            spacing: 8

            NblTextBox {
                id: titleBox
                label: "Title"
                width: parent.width
                text: root.title
            }
            NblDateInput {
                id: releaseDateBox
                label: "Release Date"
                width: parent.width
                time: root.releaseDate
            }
            NblListInput {
                id: platformList
                width: parent.width
                label: "External IDs"
                controlsOffset: titleBox.backgroundOffset

                initialItems: root.platformIds

                createNew: function() {
                    return { platform: "", id: "" }
                }
                delegate: RowLayout {
                    required property var modelData
                    width: parent.width

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
            NblButton {
                leftPadding: 24
                rightPadding: 24
                icon: ""
                text: "Search"
                activeColor: Theme.colors.accent
                inactiveColor: enabled ? Theme.colors.accent : Theme.colors.border
                enabled: searchHelper.status != MetadataSearchHelper.Status.SEARCHING
                onClicked: {

                    let platforms = {}
                    for(let i = 0 ; i < platformList.count ; i++) {
                        let modelData = platformList.itemAt(i).modelData
                        platforms[modelData.platform] = modelData.id
                    }

                    titleModel.clear()
                    descriptionModel.clear()
                    releaseModel.clear()
                    ratingModel.clear()
                    tagModel.clear()
                    posterModel.clear()
                    bannerModel.clear()
                    iconModel.clear()
                    logoModel.clear()

                    searchHelper.lookup(
                        titleBox.text, 
                        releaseDateBox.getTime(), 
                        platforms);
                }
            }
            Item {

                width: parent.width
                height: 16

                Label {
                    anchors.fill: parent
                    elide: Text.ElideRight
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    visible: searchHelper.status === MetadataSearchHelper.Status.SEARCHING
                    color: Theme.colors.textDim
                    text: "Searching..."
                }
                Label {
                    anchors.fill: parent
                    elide: Text.ElideRight
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    visible: searchHelper.status === MetadataSearchHelper.Status.DONE && root.totalResults == 0
color: Theme.colors.textDim
                    text: "No results found."
                }
            }

            // Results
            
            NblLabeledBorder { 
                label: "Posters"
                visible: posterModel.count > 0
                padding: 8
                width: parent.width
                height: posterRow.height + 16 + backgroundOffset
                Flickable {
                    height: posterRow.height
                    width: parent.width
                    clip: true
                    contentHeight: height
                    contentWidth: posterRow.width

                    Row {
                        id: posterRow
                        padding: 8
                        spacing: 16
                        Repeater {
                            id: posterRepeater
                            model: ListModel {
                                id: posterModel
                                property int selectedIndex: -1
                            }
                            delegate: NblBorderImage {
                                required property string value
                                required property int index

                                imageSource: value
                                width: 150
                                height: 225
                                sourceSize.width: 150
                                sourceSize.height: 225

                                onClicked: {
                                    if(posterModel.selectedIndex != -1 && posterModel.selectedIndex != index) {
                                        posterRepeater.itemAt(posterModel.selectedIndex).highlighted = false
                                    }
                                    highlighted = !highlighted
                                    if(highlighted) {
                                        posterModel.selectedIndex = index
                                    }
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Icons"
                visible: iconModel.count > 0
                padding: 8
                width: parent.width
                height: iconRow.height + 16 + backgroundOffset
                Flickable {
                    height: iconRow.height
                    width: parent.width
                    clip: true
                    contentHeight: height
                    contentWidth: iconRow.width
                    Row {
                        id: iconRow
                        padding: 8
                        spacing: 16
                        Repeater {
                            id: iconRepeater
                            model: ListModel {
                                id: iconModel
                                property int selectedIndex: -1
                            }
                            delegate: NblBorderImage {
                                required property string value
                                required property int index

                                imageSource: value
                                width: 150
                                height: 150
                                sourceSize.width: 150
                                sourceSize.height: 150

                                onClicked: {
                                    if(iconModel.selectedIndex != -1 && iconModel.selectedIndex != index) {
                                        iconRepeater.itemAt(iconModel.selectedIndex).highlighted = false
                                    }
                                    highlighted = !highlighted
                                    if(highlighted) {
                                        iconModel.selectedIndex = index
                                    }
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Banners"
                visible: bannerModel.count > 0
                padding: 8
                width: parent.width
                height: bannerRow.height + 16 + backgroundOffset
                Flickable {
                    height: bannerRow.height
                    width: parent.width
                    clip: true
                    contentHeight: height
                    contentWidth: bannerRow.width
                    Row {
                        id: bannerRow
                        padding: 8
                        spacing: 16
                        Repeater {
                            id: bannerRepeater
                            model: ListModel {
                                id: bannerModel
                                property int selectedIndex: -1
                            }
                            delegate: NblBorderImage {
                                required property string value
                                required property int index

                                imageSource: value
                                width: 96 * 3.25
                                height: 31 * 3.25
                                sourceSize.width: width
                                sourceSize.height: height

                                onClicked: {
                                    if(bannerModel.selectedIndex != -1 && bannerModel.selectedIndex != index) {
                                        bannerRepeater.itemAt(bannerModel.selectedIndex).highlighted = false
                                    }
                                    highlighted = !highlighted
                                    if(highlighted) {
                                        bannerModel.selectedIndex = index
                                    }
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Logos"
                visible: logoModel.count > 0
                padding: 8
                width: parent.width
                height: logoRow.height + 16 + backgroundOffset
                Flickable {
                    height: logoRow.height
                    width: parent.width
                    clip: true
                    contentHeight: height
                    contentWidth: logoRow.width
                    Row {
                        id: logoRow
                        padding: 8
                        spacing: 16
                        Repeater {
                            id: logoRepeater
                            model: ListModel {
                                id: logoModel
                                property int selectedIndex: -1
                            }
                            delegate: NblBorderImage {
                                required property string value
                                required property int index

                                imageSource: value
                                width: 150
                                height: 150
                                sourceSize.width: width
                                sourceSize.height: height
                                fillMode: Image.PreserveAspectFit

                                onClicked: {
                                    if(logoModel.selectedIndex != -1 && logoModel.selectedIndex != index) {
                                        logoRepeater.itemAt(logoModel.selectedIndex).highlighted = false
                                    }
                                    highlighted = !highlighted
                                    if(highlighted) {
                                        logoModel.selectedIndex = index
                                    }
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Tags"
                visible: tagModel.count > 0
                padding: 16
                width: parent.width
                height: tagColumn.height + 32 + backgroundOffset
                Column {
                    id: tagColumn
                    width: parent.width
                    spacing: 8
                    Flow {
                        width: parent.width
                        spacing: 8
                        Repeater {
                            model: ListModel {
                                id: addedTagModel
                            }
                            delegate: NblTag {
                                required property int index
                                required property int tagId
                                required property int tagModelIndex
                                onClicked: {
                                    addedTagModel.remove(index)
                                    tagRepeater.itemAt(tagModelIndex).visible = true
                                }
                                Rectangle {
                                    width: 12
                                    height: 12
                                    radius: 6
                                    color: Theme.colors.accent
                                    x: parent.width - 6
                                    y: -6
                                }
                            }
                        }
                    }
                    Rectangle {
                        visible: addedTagModel.count > 0
                        width: parent.width
                        height: 2
                        color: Theme.colors.border
                    }
                    Flow {
                        width: parent.width
                        spacing: 8
                        Repeater {
                            id: tagRepeater
                            model: ListModel {
                                id: tagModel
                            }
                            delegate: NblTag {
                                required property string value
                                required property int index

                                property bool added: false
                                property int tagId: 0

                                text: value
                                color: "white"

                                Component.onCompleted: {
                                    tagId = ApplicationContext.findTagId(value)
                                    if(tagId == 0) {
                                        added = true
                                        color = Theme.colors.tagColor(value)
                                    } else {
                                        let tagMeta = ApplicationContext.getTagsFromIds([tagId])
                                        color = tagMeta[0].color
                                    }
                                }

                                onClicked: {
                                    addedTagModel.append({ tagId: tagId, text: text, color: color, tagModelIndex: index })
                                    visible = false
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Alternate Titles"
                visible: titleModel.count > 0
                padding: 16
                width: parent.width
                height: titleColumn.height + 32 + backgroundOffset
                Column {
                    id: titleColumn
                    width: parent.width
                    spacing: 8
                    Repeater {
                        id: titleRepeater
                        model: ListModel {
                            id: titleModel
                            property int selectedIndex: -1
                        }
                        delegate: NblCheckBox {
                            required property string value
                            required property int index
                            text: value
                            elide: Text.ElideRight
                            width: parent.width
                            checked: titleModel.selectedIndex == index
                            onToggled: {
                                if(titleModel.selectedIndex != -1) titleRepeater.itemAt(titleModel.selectedIndex).checked = false
                                if(checked) {
                                    titleModel.selectedIndex = index
                                } else {
                                    titleModel.selectedIndex = -1
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Descriptions"
                visible: descriptionModel.count > 0
                padding: 16
                width: parent.width
                height: descriptionColumn.height + 32 + backgroundOffset
                Column {
                    id: descriptionColumn
                    width: parent.width
                    Repeater {
                        id: descriptionRepeater
                        model: ListModel {
                            id: descriptionModel
                            property int selectedIndex: -1
                        }
                        // TODO: Make this expandable.
                        delegate: NblCheckBox {
                            required property string value
                            required property int index
                            text: value
                            width: parent.width
                            elide: Text.ElideRight
                            checked: descriptionModel.selectedIndex == index
                            onToggled: {
                                if(descriptionModel.selectedIndex != -1) descriptionRepeater.itemAt(descriptionModel.selectedIndex).checked = false
                                if(checked) {
                                    descriptionModel.selectedIndex = index
                                } else {
                                    descriptionModel.selectedIndex = -1
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Release Dates"
                visible: releaseModel.count > 0
                padding: 16
                width: parent.width
                height: releaseColumn.height + 32 + backgroundOffset
                Column {
                    id: releaseColumn
                    width: parent.width
                    Repeater {
                        id: releaseRepeater
                        model: ListModel {
                            id: releaseModel
                            property int selectedIndex: -1
                        }
                        delegate: NblCheckBox {
                            required property var value
                            required property int index
                            text: Theme.formatDate(value)
                            width: parent.width
                            elide: Text.ElideRight
                            checked: releaseModel.selectedIndex == index
                            onToggled: {
                                if(releaseModel.selectedIndex != -1) releaseRepeater.itemAt(releaseModel.selectedIndex).checked = false
                                if(checked) {
                                    releaseModel.selectedIndex = index
                                } else {
                                    releaseModel.selectedIndex = -1
                                }
                            }
                        }
                    }
                }
            }

            NblLabeledBorder { 
                label: "Ratings"
                visible: ratingModel.count > 0
                padding: 16
                width: parent.width
                height: releaseColumn.height + 32 + backgroundOffset
                Column {
                    id: ratingColumn
                    width: parent.width
                    Repeater {
                        id: ratingRepeater
                        model: ListModel {
                            id: ratingModel
                            property int selectedIndex: -1
                        }
                        delegate: NblCheckBox {
                            required property var value
                            required property int index
                            text: value + "%"
                            width: parent.width
                            elide: Text.ElideRight
                            checked: ratingModel.selectedIndex == index
                            onToggled: {
                                if(ratingModel.selectedIndex != -1) ratingRepeater.itemAt(ratingModel.selectedIndex).checked = false
                                if(checked) {
                                    ratingModel.selectedIndex = index
                                } else {
                                    ratingModel.selectedIndex = -1
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
