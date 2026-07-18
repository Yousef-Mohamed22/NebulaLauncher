pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

Item {

    id: root

    property bool modified

    function refresh() {
        searchModel.refresh()
        flickable.contentY = 0
    }

    RowLayout {

        width: parent.width
        height: 54

        NblTextBox {
            id: filterBox
            Layout.fillWidth: true
            label: "Filter"
        }

        NblButton {

            id: newBtn

            icon: ""
            text: "New"

            Layout.preferredWidth: 128
            Layout.preferredHeight: 48
            Layout.topMargin: 6

            inactiveColor: Theme.colors.layer1
            centered: true

            onClicked: {
                if(root.modified) {
                    ApplicationContext.pushNotification({
                        type: "error",
                        title: "Cannot Add Tag",
                        message: "Please apply your changes first."
                    })
                    return
                }

                newPopup.open()
            }
        }
    }

    Popup {
        id: newPopup

        width: 350
        height: 316

        y: newBtn.y + newBtn.height + 8
        x: newBtn.x

        padding: 8

        background: Rectangle {
            color: Theme.colors.layer0
            radius: 8
            border.width: 2
            border.color: Theme.colors.border
        }

        contentItem: Column {
            id: newTagInfo
            width: parent.width
            spacing: 8
            NblTextBox {
                id: newTagNameBox
                label: "Name"
                width: parent.width

                onTextEdited: {
                    text = text.toLowerCase().replace(' ', '_')
                }

            }

            NblColorSwatch {
                id: newTagColorBox
                label: "Color"
                width: 96
                height: 48
                selectedColor: Qt.hsva(Math.random(), 0.6 + Math.random() * 0.3, 0.4 + Math.random() * 0.5)
            }

            NblTagList {
                id: newParentsBox
                label: "Parents"
                width: parent.width
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
                    text: "OK"
                }

                MouseArea {
                    id: addMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if(newTagNameBox.text.length == 0) {
                            ApplicationContext.pushNotification({
                                type: "error",
                                title: "Cannot Add Tag",
                                message: "Name field must not be empty!"
                            })
                            return
                        }
                        ApplicationContext.addTag(newTagNameBox.text, newTagColorBox.selectedColor)

                        let parents = newParentsBox.getAddedTags()
                        for(let i = 0 ; i < parents.length ; i++) {
                            console.log("Adding tag parent:", parents[i])
                            ApplicationContext.addTagParent(newTagNameBox.text, parents[i])
                        }

                        root.refresh()
                        newPopup.close()
                    }
                }
            }
        }
    }


    function encodeModifiedTags() {

        let output = []

        for(let i = 0 ; i < tagRepeater.count ; i++) {
            let tagElement = tagRepeater.itemAt(i)
            if(tagElement.modified && !tagElement.removed) {
                output.push(tagElement.encodeTagInfo())
            }
        }

        return output
    }

    function getRemovedTags() {
        let output = []
        for(let i = 0 ; i < tagRepeater.count ; i++) {
            let tagElement = tagRepeater.itemAt(i)
            if(tagElement.removed) {
                output.push(tagElement.getOriginalName())
            }
        }
        return output
    }

    Flickable {

        width: root.width + 8
        height: parent.height - 64
        y: 64

        id: flickable

        contentWidth: root.width
        contentHeight: tagColumn.height
        flickableDirection: Flickable.VerticalFlick
        clip: true

        ScrollBar.horizontal: null
        ScrollBar.vertical: ScrollBar {
            parent: flickable
            anchors.top: flickable.top
            anchors.bottom: flickable.bottom
            anchors.right: flickable.right
            active: flickable.movingVertically
        }

        onAtYEndChanged: {
            if(atYEnd) {
                searchModel.loadNextPage()
            }
        }

        Column {

            id: tagColumn

            Repeater {

                id: tagRepeater

                model: NblTagSearchModel {
                    id: searchModel
                    searchQuery: filterBox.text
                    pageSize: 10
                }

                delegate: Item { 

                    width: root.width
                    height: dropdown.height

                    id: tagElement
                    
                    required property var modelData
                    property bool modified: false
                    property bool removed: false

                    onModifiedChanged: {
                        if(modified) root.modified = true
                    }

                    function getOriginalName() {
                        return modelData.name
                    }

                    function encodeTagInfo() {
                        return {
                            originalName: tagElement.modelData.name,
                            name: tagNameBox.text,
                            color: tagColorBox.selectedColor,
                            addedParents: parentsBox.getAddedTags(),
                            removedParents: parentsBox.getRemovedTags()
                        }
                    }

                    NblDropdown {

                        id: dropdown

                        width: parent.width - parent.height
                        label: tagElement.modelData.name

                        contentHeight: tagInfo.height
                        enabled: !tagElement.removed

                        property bool loadedTags: false
                        onCollapsedChanged: {
                            if(!collapsed && !loadedTags) {
                                loadedTags = true
                                parentsBox.initialTags = ApplicationContext.getTagParents(tagElement.modelData.name)
                            }
                        }

                        Column {
                            id: tagInfo
                            width: parent.width
                            NblTextBox {
                                id: tagNameBox
                                label: "Name"
                                width: parent.width
                                text: tagElement.modelData.name
                                onTextEdited: {
                                    text = text.toLowerCase().replace(' ', '_')
                                    tagElement.modified = true
                                }
                            }

                            NblColorSwatch {
                                id: tagColorBox
                                label: "Color"
                                width: 96
                                height: 48
                                selectedColor: tagElement.modelData.color
                                onSelectedColorChanged: {
                                    if(selectedColor != tagElement.modelData.color) {
                                        tagElement.modified = true
                                    }
                                }
                            }

                            NblTagList {
                                id: parentsBox
                                label: "Parents"
                                width: parent.width
                                onModified: {
                                    tagElement.modified = true
                                }
                            }
                        }
                    }

                    Item {
                        width: dropdown.collapsedHeight
                        height: width

                        x: tagElement.width - width

                        Rectangle {
                            anchors.fill: parent
                            color: removeArea.containsMouse ? Theme.colors.layer1 : Theme.colors.layer0
                            radius: 8
                        }

                        Label {
                            text: tagElement.removed ? "" : ""
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
                                tagElement.removed = !tagElement.removed
                            }
                        }
                    }

                    Rectangle {
                        width: dropdown.width
                        height: 8
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.colors.layer0
                        visible: tagElement.removed
                    }

                    Rectangle {
                        width: dropdown.width
                        height: 2
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.colors.textDim
                        visible: tagElement.removed
                    }

                }
            }
        }
    }
}

