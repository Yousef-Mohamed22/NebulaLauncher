pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    NblTextBox {
        id: filterBox
        width: parent.width
        label: "Filter"
    }

    function encodeTags() {

        let output = []

        for(let i = 0 ; i < tagRepeater.count ; i++) {
            let dropdown = tagRepeater.itemAt(i)
            if(dropdown.modified) {
                output.push(dropdown.encodeTagInfo())
            }
        }

        return output
    }

    Flickable {

        width: parent.width
        height: parent.height - 64
        y: 64

        id: flickable

        contentWidth: width
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

                delegate: NblDropdown {

                    width: root.width

                    id: dropdown
                    required property var modelData
                    property bool modified: false

                    label: modelData.name

                    contentHeight: tagInfo.height

                    function encodeTagInfo() {
                        return {
                            originalName: modelData.name,
                            name: tagNameBox.text,
                            color: tagColorBox.selectedColor,
                            addedParents: parentsBox.getAddedTags(),
                            removedParents: parentsBox.getRemovedTags()
                        }
                    }

                    Column {
                        id: tagInfo
                        width: parent.width
                        NblTextBox {
                            id: tagNameBox
                            label: "Name"
                            width: parent.width
                            text: dropdown.modelData.name
                            onTextEdited: {
                                dropdown.modified = true
                            }
                        }

                        NblColorSwatch {
                            id: tagColorBox
                            label: "Color"
                            width: 96
                            height: 48
                            selectedColor: dropdown.modelData.color
                            onSelectedColorChanged: {
                                if(selectedColor != dropdown.modelData.color) {
                                    dropdown.modified = true
                                }
                            }
                        }

                        NblTagList {
                            id: parentsBox
                            label: "Parents"
                            width: parent.width
                            onModified: {
                                dropdown.modified = true
                            }
                            // todo: initialTags
                        }
                    }
                }
            }
        }
    }
}

