pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Nebula.Controls

NblDialog {
    
    id: root

    showApplyButton: false

    function encode() {

        let filters = []
        for(let i = 0 ; i < filterList.count; i++) {
            let filter = filterList.itemAt(i)
            filters.push(filter.encode())
        }

        return {
            name: nameInput.text,
            icon: iconPicker.icon,
            colorizeIcon: iconPicker.colorize,
            hidden: hiddenBox.checked,
            filters: filters
        }
    }

    onApply: {
        console.log(JSON.stringify(encode()))
    }

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

                NblTextBox {
                    id: nameInput
                    width: parent.width
                    label: "Name"
                }

                NblIconPicker {
                    id: iconPicker
                }

                NblCheckBox {
                    id: hiddenBox
                    width: parent.width
                    text: "Hidden"
                }

                
                NblListInput {

                    id: filterList
                    label: "Filters"
                    width: parent.width

                    createNew: function(index) {
                        return "Filter"
                    }

                    delegate: NblDropdown {

                        id: dropdown

                        required property var modelData
                        width: parent.width

                        contentHeight: filterContent.height
                        label: filterNameBox.text

                        function encode() {
                            return {
                                name: filterNameBox.text,
                                icon: filterIconPicker.icon,
                                colorizeIcon: filterIconPicker.colorize,
                                multiLine: multiLineBox.checked,
                                useIcons: useIconsBox.checked,
                                showTitles: showTitlesBox.checked,
                                sortBy: sortByBox.currentIndex,
                                sortOrder: sortByBox.currentIndex,
                                tags: tagSetsList.encode(),
                                excludeTags: excludeTagsList.encode(),
                                minReleaseDate: minReleaseBox.getTime(),
                                maxReleaseDate: maxReleaseBox.getTime(),
                                minRating: minRatingBox.value,
                                maxRating: maxRatingBox.value,
                                minTime: minTimeBox.value,
                                maxTime: maxTimeBox.value
                            }
                        }

                        Column {

                            width: parent.width

                            id: filterContent
                            spacing: 8


                            NblTextBox {
                                id: filterNameBox
                                width: parent.width
                                label: "Name"
                                text: dropdown.modelData
                            }

                            NblIconPicker {
                                id: filterIconPicker
                            }

                            RowLayout {
                                width: parent.width

                                NblCheckBox {
                                    id: multiLineBox
                                    Layout.fillWidth: true
                                    text: "Multi-line"
                                }
                                NblCheckBox {
                                    id: useIconsBox
                                    Layout.fillWidth: true
                                    text: "Use Icons"
                                }
                                NblCheckBox {
                                    id: showTitlesBox
                                    Layout.fillWidth: true
                                    text: "Show Titles"
                                }
                            }

                            NblComboBox {
                                id: sortByBox
                                width: parent.width
                                label: "Sort By"
                                model: [ "Title", "Last Played", "Added", "Play Count", "Play Time", "Release Date", "Rating" ]
                            }

                            NblComboBox {
                                id: sortOrderBox
                                width: parent.width
                                label: "Sort Order"
                                model: [ "Ascending", "Descending" ]
                            }

                            NblListInput {
                                id: tagSetsList
                                label: "Tag Sets"
                                width: parent.width
                                createNew: function(i) {
                                    return i + 1
                                }
                                delegate: NblTagList {
                                    required property var modelData
                                    width: parent.width
                                    labelIndent: 24
                                    label: "Tag Set " + modelData
                                }

                                function encode() {

                                    let out = []

                                    for(let i = 0 ; i < count ; i ++) {

                                        let set = []

                                        let tagList = itemAt(i) as NblTagList
                                        let tags = tagList.getListedTags()
                                        for(let t = 0 ; t < tags.length ; t++) {
                                            set.push(tags[t])
                                        }

                                        out.push(set)
                                    }

                                    return out
                                }
                            }

                            NblTagList {
                                id: excludeTagsList
                                width: parent.width
                                label: "Exclude Tags"

                                function encode() {

                                    let set = []
                                    let tags = getListedTags()
                                    for(let t = 0 ; t < tags.length ; t++) {
                                        set.push(tags[t])
                                    }

                                    return set
                                }
                            }

                            NblDateInput {
                                id: minReleaseBox
                                width: parent.width
                                label: "Minimum Release Date"
                            }

                            NblDateInput {
                                id: maxReleaseBox
                                width: parent.width
                                label: "Maximum Release Date"
                            }

                            NblNumberInput {
                                id: minRatingBox
                                width: parent.width
                                label: "Minimum Rating"
                                min: 0
                                value: 0
                            }

                            NblNumberInput {
                                id: maxRatingBox
                                width: parent.width
                                label: "Maximum Rating"
                                min: -1
                                value: -1
                            }

                            NblNumberInput {
                                id: minTimeBox
                                width: parent.width
                                label: "Minimum Play Time (ms)"
                                min: 0
                                value: 0
                            }

                            NblNumberInput {
                                id: maxTimeBox
                                width: parent.width
                                label: "Maximum Play Time (ms)"
                                min: -1
                                value: -1
                            }
                        }
                    }
                }
            }
        }
    }
}
