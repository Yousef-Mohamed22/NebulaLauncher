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

            Column {
                id: tabList
                width: parent.width
                spacing: 8

                Repeater {
                    model: ApplicationContext.allTabs

                    NblSidebarButton {
                        required property NblTabRef modelData

                        width: parent.width

                        collapsed: root.collapsed
                        icon: modelData.icon
                        name: modelData.name

                        onClicked: {
                            root.mainWindow.openTab(modelData)
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

                    Popup {
                        id: addMenu
                        visible: false

                        width: 200
                        height: 96

                        x: parent.width + 16
                        y: parent.height - height + 8

                        margins: 0
                        padding: 2

                        background: Rectangle {
                            color: Theme.colors.layer1
                            border.width: 2
                            border.color: Theme.colors.border
                            radius: 8
                        }

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
