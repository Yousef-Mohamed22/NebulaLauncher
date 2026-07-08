pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    required property MainWindow mainWindow
    property bool collapsed: true

    implicitWidth: collapsed ? 64 : 240

    Component.onCompleted: {
        if(tabRepeater.count > 0) {
            mainWindow.openTab((tabRepeater.itemAt(0) as NblTabButton).tabId)
        }
    }

    function reloadTabs() {
        tabsModel.refresh()
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.colors.layer1
        border.width: 2
        border.color: Theme.colors.border
        radius: 8
    }

    ColumnLayout {
        width: parent.width - 16
        height: parent.height - 16
        anchors.centerIn: parent

        spacing: 8

        Flickable {

            id: flickable

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            interactive: false

            contentWidth: width
            contentHeight: tabList.height

            flickableDirection: Flickable.VerticalFlick

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
                    tabsModel.loadNextPage()
                }
            }

            Column {
                id: tabList
                width: parent.width
                spacing: 8

                Repeater {

                    id: tabRepeater

                    model: NblAllTabsModel { 
                        pageSize: 20
                        id: tabsModel
                    }

                    NblTabButton {
                        required property var modelData

                        tabId: modelData.id
                        width: parent.width

                        collapsed: root.collapsed
                        icon: modelData.icon
                        name: modelData.name
                        colorize: modelData.colorizeIcon

                        onClicked: {
                            root.mainWindow.openTab(tabId)
                        }
                        onEditRequested: {
                            root.mainWindow.beginEditTab(tabId)
                        }
                        onDeleteRequested: {
                            root.mainWindow.openDialog("Pages/ConfirmDeleteDialog.qml", {
                                "text": "Are you sure you want to delete this tab?"
                            }, function() {
                                ApplicationContext.deleteTab(tabId)
                                root.reloadTabs()
                                if(tabRepeater.count > 0) {
                                    root.mainWindow.openTab((tabRepeater.itemAt(0) as NblTabButton).tabId)
                                }
                            })
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            implicitHeight: root.collapsed ? 48 + 48 + 8 : 48

            FlexboxLayout {
                gap: 8
                width: parent.width
                height: parent.height
                direction: root.collapsed ? FlexboxLayout.Column : FlexboxLayout.Row

                NblSidebarButton {

                    Layout.preferredWidth: 48
                    collapsed: true
                    icon: ""
                    name: ""

                    onClicked: { 
                        root.mainWindow.openSettings()
                    }
                }

                NblSidebarButton {

                    Layout.fillWidth: true

                    collapsed: root.collapsed
                    icon: ""
                    name: "Add"
                    alignment: Text.AlignHCenter

                    onClicked: { 
                        addMenu.open()
                    }

                    NblContextMenu {
                        id: addMenu
                        height: 96
                        contentItem: ColumnLayout {
                            spacing: 0
                            NblButton {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                icon: "󰮂"
                                text: "Add Installation"
                                onClicked: {
                                    addMenu.close()
                                    root.mainWindow.beginAddGame()
                                }
                                radius: 2
                            }
                            NblButton {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                icon: "󰓩"
                                text: "Add Tab"
                                onClicked: {
                                    addMenu.close()
                                    root.mainWindow.beginAddTab()
                                }
                                radius: 2
                            }
                        }
                    }
                }
            }
        }
    }
}
