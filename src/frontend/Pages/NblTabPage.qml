pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    required property int tabId
    required property MainWindow mainWindow

    ScrollView {

        id: scrollView
        width: parent.width
        height: parent.height - 8
        y: 8

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        contentHeight: content.height

        Flickable {

            anchors.fill: parent
            interactive: true
            clip: true

            id: flickable

            Column {

                id: content

                width: root.width
                spacing: 8

                Repeater {
                    model: NblTabModel {
                        id: tabModel
                        pageSize: 10
                        tabId: root.tabId
                    }
                    delegate: NblFilterRow {

                        id: row
                        required property var modelData
                        width: root.width - 8

                        filterId: modelData.id
                        name: modelData.name
                        icon: modelData.icon
                        colorizeIcon: modelData.colorizeIcon
                        multiLine: modelData.multiLine
                        showTitles: modelData.showTitles
                        useIcons: modelData.useIcons

                        onInstallationClicked: id => {
                            root.mainWindow.openInstallationDetails(id)
                        }

                        Connections {
                            target: flickable
                            function onContentYChanged() {
                                row.scrolled()
                            }
                        }
                    }
                }
            }
        }
    }

    function refresh() {
        tabModel.refresh()
    }

}
