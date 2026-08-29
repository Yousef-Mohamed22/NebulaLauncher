pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    property bool collapsed: true
    property int selectedTab: 0

    signal tabClicked(int tabId)
    signal tabEdited(int tabId)
    signal gameAdded()

    implicitWidth: collapsed ? 64 : 240

    Component.onCompleted: {
        if(tabRepeater.count > 0) {
            root.tabClicked((tabRepeater.itemAt(0) as NblTabButton).tabId)
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

                    delegate: NblTabButton {
                        required property var modelData

                        tabId: modelData.id
                        width: parent.width

                        collapsed: root.collapsed
                        icon: modelData.icon
                        name: modelData.name
                        colorize: modelData.colorizeIcon

                        onClicked: {
                            root.selectedTab = tabId
                            root.tabClicked(tabId)
                        }
                        onEditRequested: {
                            let dialogComponent = Qt.createComponent("AddTabDialog.qml")
                            let dialog = dialogComponent.createObject(root, { tabId: tabId }) as AddTabDialog
                            dialog.apply.connect(function() {
                                root.reloadTabs()
                                root.tabEdited(tabId)
                            })
                            dialog.open()
                        }
                        onDeleteRequested: {
                            let dialogComponent = Qt.createComponent("ConfirmDeleteDialog.qml")
                            let dialog = dialogComponent.createObject(this, { 
                                "text": "Are you sure you want to delete this tab?" 
                            }) as ConfirmDeleteDialog
                            dialog.apply.connect(function() {
                                root.reloadTabs()
                                root.tabEdited(tabId)
                            })
                            dialog.open()
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
                        let dialogComponent = Qt.createComponent("AppSettingsDialog.qml")
                        let dialog = dialogComponent.createObject(root, { }) as AppSettingsDialog
                        dialog.open()
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
                                icon: "󰊗"
                                text: "Add Installation"
                                onClicked: {
                                    addMenu.close()
                                    let dialogComponent = Qt.createComponent("AddGameDialog.qml")
                                    let dialog = dialogComponent.createObject(root, {}) as NblDialog
                                    dialog.apply.connect(function() {
                                        root.gameAdded()
                                    })
                                    dialog.open()
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
                                    let dialogComponent = Qt.createComponent("AddTabDialog.qml")
                                    let dialog = dialogComponent.createObject(root, {}) as NblDialog
                                    dialog.apply.connect(function() {
                                        root.reloadTabs()
                                    })
                                    dialog.open()
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
