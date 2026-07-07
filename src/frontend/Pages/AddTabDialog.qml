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
                        return {
                            id: 0,
                            name: "Filter",
                            icon: "󰊗",
                            colorizeIcon: true,
                            multiLine: true,
                            useIcons: false,
                            showTitles: false,
                            sortBy: 0,
                            sortOrder: 0,
                            tags: [],
                            excludeTags: [],
                            minReleaseDate: 0,
                            maxReleaseDate: 0,
                            minRating: 0,
                            maxRating: 0,
                            minTime: 0,
                            maxTime: 0
                        }
                    }

                    delegate: NblDropdown {

                        id: dropdown

                        required property var modelData
                        width: parent.width

                        contentHeight: filterContent.height
                        label: filterNameBox.text

                        function encode() {
                            return {
                                id: modelData.id,
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
                                text: dropdown.modelData.name
                            }

                            NblIconPicker {
                                id: filterIconPicker
                                icon: dropdown.modelData.icon
                                colorize: dropdown.modelData.colorizeIcon
                            }

                            RowLayout {
                                width: parent.width

                                NblCheckBox {
                                    id: multiLineBox
                                    Layout.fillWidth: true
                                    text: "Multi-line"
                                    checked: dropdown.modelData.multiLine
                                }
                                NblCheckBox {
                                    id: useIconsBox
                                    Layout.fillWidth: true
                                    text: "Use Icons"
                                    checked: dropdown.modelData.useIcons
                                }
                                NblCheckBox {
                                    id: showTitlesBox
                                    Layout.fillWidth: true
                                    text: "Show Titles"
                                    checked: dropdown.modelData.showTitles
                                }
                            }

                            NblComboBox {
                                id: sortByBox
                                width: parent.width
                                label: "Sort By"
                                model: [ "Title", "Last Played", "Added", "Play Count", "Play Time", "Release Date", "Rating" ]
                                currentIndex: dropdown.modelData.sortBy
                            }

                            NblComboBox {
                                id: sortOrderBox
                                width: parent.width
                                label: "Sort Order"
                                model: [ "Ascending", "Descending" ]
                                currentIndex: dropdown.modelData.sortOrder
                            }

                            NblListInput {
                                id: tagSetsList
                                label: "Tag Sets"
                                width: parent.width

                                initialItems: getInitialItems()

                                function getInitialItems() {
                                    let tagLists = dropdown.modelData.tags
                                    let out = []
                                    for(let i = 0 ; i < tagLists.length ; i++) {
                                        out.push({
                                            index: i + 1,
                                            initialTags: tagLists[i]
                                        })
                                    }
                                    return out
                                }

                                createNew: function(i) {
                                    return {
                                        index: i + 1,
                                        initialTags: []
                                    }
                                }
                                delegate: NblTagList {
                                    required property var modelData
                                    width: parent.width
                                    labelIndent: 24
                                    label: "Tag Set " + modelData.index
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
                                initialTags: dropdown.modelData.excludeTags

                                function encode() {

                                    let set = []
                                    let tags = getListedTags()
                                    for(let t = 0 ; t < tags.length ; t++) {
                                        set.push(tags[t])
                                    }

                                    return set
                                }
                            }

                            function getDate(time) {
                                let out = new Date()
                                out.setTime(time)
                                return out
                            }

                            NblDateInput {
                                id: minReleaseBox
                                width: parent.width
                                label: "Minimum Release Date"
                                date: parent.getDate(dropdown.modelData.minReleaseDate)
                            }

                            NblDateInput {
                                id: maxReleaseBox
                                width: parent.width
                                label: "Maximum Release Date (exclusive)"
                                date: parent.getDate(dropdown.modelData.maxReleaseDate)
                            }

                            NblNumberInput {
                                id: minRatingBox
                                width: parent.width
                                label: "Minimum Rating"
                                min: 0
                                max: 100
                                allowDecimal: true
                                value: dropdown.modelData.minRating
                            }

                            NblNumberInput {
                                id: maxRatingBox
                                width: parent.width
                                label: "Maximum Rating (exclusive)"
                                min: 0
                                max: 100
                                allowDecimal: true
                                value: dropdown.modelData.minRating
                            }

                            NblNumberInput {
                                id: minTimeBox
                                width: parent.width
                                label: "Minimum Play Time (ms)"
                                min: 0
                                value: dropdown.modelData.minTime
                            }

                            NblNumberInput {
                                id: maxTimeBox
                                width: parent.width
                                label: "Maximum Play Time (ms) (exclusive)"
                                min: 0
                                value: dropdown.modelData.maxTime
                            }
                        }
                    }
                }
            }
        }
    }
}
