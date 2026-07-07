pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    required property NblTabRef tab
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
                        tabId: root.tab.tabId
                    }
                    delegate: NblFilterRow {

                        id: row
                        required property var model
                        width: root.width - 8

                        filterId: model.id
                        name: model.name
                        icon: model.icon
                        colorizeIcon: model.colorizeIcon
                        multiLine: model.multiLine
                        showTitles: model.showTitles
                        useIcons: model.useIcons

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
