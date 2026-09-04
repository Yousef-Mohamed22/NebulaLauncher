pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

NblDialog {

    id: root

    padding: 0
    navigationPadding: 16

    targetWidth: 960

    showCancelButton: sidebar.currentTab.showCancelButton
    showApplyButton: sidebar.currentTab.showApplyButton

    Item {

        anchors.fill: parent

        property int sidebarWidth: 200

        Item {
            id: sidebar
            width: parent.sidebarWidth
            height: parent.height + root.navigationHeight + root.navigationGap

            property SettingsTab currentTab: themeSettingsTab

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
                            { icon: "", name: "Theme", tab: themeSettingsTab },
                            { icon: "", name: "Tags", tab: tagSettingsTab },
                            { icon: "󰊗", name: "Games", tab: gameSettingsTab },
                            { icon: "", name: "Online Accounts", tab: accountSettingsTab },
                            { icon: "", name: "Plugins", tab: pluginsTab }
                        ]

                        delegate: Item {

                            required property var modelData

                            readonly property bool isActive: sidebar.currentTab == modelData.tab
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
                                    if(parent.modelData.tab == sidebar.currentTab) return
                                    if(sidebar.currentTab.modified) {
                                        ApplicationContext.pushNotification({
                                            type: "error",
                                            title: "Cannot Change Tabs",
                                            message: "Please apply your changes first."
                                        })
                                        return
                                    }

                                    sidebar.currentTab.modified = false
                                    sidebar.currentTab.visible = false

                                    sidebar.currentTab = parent.modelData.tab
                                    sidebar.currentTab.refresh()
                                    sidebar.currentTab.visible = true
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
                visible: true
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

            AccountsSettingsTab {
                id: accountSettingsTab
                anchors.fill: parent
                visible: false
            }

           PluginSettingsTab {
                id: pluginsTab
                anchors.fill: parent
                visible: false
            }
        }

    }

    onApply: {
        sidebar.currentTab.apply()
    }

}
