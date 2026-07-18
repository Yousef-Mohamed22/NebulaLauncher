pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

NblDialog {

    id: root

    padding: 0
    navigationPadding: 16

    targetWidth: 960

    Item {

        anchors.fill: parent

        property int sidebarWidth: 200

        Item {
            id: sidebar
            width: parent.sidebarWidth
            height: parent.height + root.navigationHeight + root.navigationGap

            property Item currentItem: themeSettingsTab

            Rectangle { 
                width: 2
                height: parent.height - 32
                x: parent.width
                y: 16
                color: Theme.colors.border
            }

            Flickable {

                anchors.fill: parent
                anchors.topMargin: 16

                id: tabFlickable

                contentWidth: width
                contentHeight: themeSettingsTab.height
                flickableDirection: Flickable.VerticalFlick
                clip: true

                ScrollBar.horizontal: null
                ScrollBar.vertical: ScrollBar {
                    parent: tabFlickable
                    anchors.top: tabFlickable.top
                    anchors.bottom: tabFlickable.bottom
                    anchors.right: tabFlickable.right
                    active: tabFlickable.movingVertically
                }

                Column {

                    width: parent.width

                    Repeater {
                        model: [
                            { icon: "", name: "Theme", item: themeSettingsTab },
                            { icon: "", name: "Tags", item: tagSettingsTab },
                            { icon: "󰊗", name: "Games", item: gameSettingsTab }
                        ]

                        delegate: Item {

                            required property var modelData

                            readonly property bool isActive: sidebar.currentItem == modelData.item
                            readonly property bool isHighlighted: isActive || tabArea.containsMouse

                            width: parent.width
                            height: 48

                            Rectangle {
                                anchors.fill: parent
                                color: parent.isActive
                                        ? Theme.colors.border
                                        : parent.isHighlighted ? Theme.colors.layer1 : "transparent"
                            }

                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                x: 24
                                text: parent.modelData.icon
                                font.family: Theme.nerdFont
                                font.pixelSize: 24
                                color: parent.isHighlighted ? Theme.colors.text : Theme.colors.textDim
                            }

                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                x: 64
                                text: parent.modelData.name
                                font.family: Theme.font
                                font.pixelSize: 16
                                color: parent.isHighlighted ? Theme.colors.text : Theme.colors.textDim
                            }

                            MouseArea {
                                id: tabArea
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                anchors.fill: parent
                                onClicked: {
                                    if(parent.modelData.item == sidebar.currentItem) return
                                    sidebar.currentItem.visible = false
                                    sidebar.currentItem = parent.modelData.item
                                    sidebar.currentItem.visible = true
                                }
                            }

                        }
                    }
                }
            }
        }
        
        Item {

            width: parent.width - parent.sidebarWidth - 48
            height: parent.height - 48
            x: parent.sidebarWidth + 24
            y: 24

            ThemeSettingsTab {
                id: themeSettingsTab
                anchors.fill: parent
                visible: true;
            }

            TagSettingsTab {
                id: tagSettingsTab
                anchors.fill: parent
                visible: false
                onVisibleChanged: {
                    if(visible) {
                        paused = false
                    }
                }
            }

            GameSettingsTab {
                id: gameSettingsTab
                anchors.fill: parent
                visible: false
            }
        }

    }

    onApply: {
        Theme.colors.lightMode = !themeSettingsTab.darkTheme
        Theme.colors.activeColor = themeSettingsTab.selectedColor

        let res = ApplicationContext.updateTags(tagSettingsTab.encodeModifiedTags())
        if(res != 0) {
            ApplicationContext.pushNotification({
                type: "error",
                title: "Failed to Update Tags",
                message: "An error occurred. This is usually due to cycles in a tag's ancestry."
            })
            console.log("An error occurred while updating tags!", res)
        }

        let removedTags = tagSettingsTab.getRemovedTags()
        for(let i = 0 ; i < removedTags.length ; i++) {
            let res = ApplicationContext.removeTag(removedTags[i])
            if(res != 0) {
                ApplicationContext.pushNotification({
                    type: "error",
                    title: "Failed to Remove a Tag",
                    message: "An error occurred while removing the tag: \"" + removedTags[i] + "\""
                })
                console.log("An error occurred while removing a tag!", res) 
            }
        }

        tagSettingsTab.refresh()
    }

}
