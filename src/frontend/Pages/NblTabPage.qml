pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    required property int tabId
    required property MainWindow mainWindow

    NblScrollView {

        id: scrollView
        width: parent.width + 16
        height: parent.height - 8
        y: 8
        clip: true

        contentHeight: content.height
        contentWidth: width

        Column {

            id: content

            width: scrollView.contentWidth
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
                    width: content.width - 8

                    filterId: modelData.id
                    name: modelData.name
                    icon: modelData.icon
                    colorizeIcon: modelData.colorizeIcon
                    multiLine: modelData.multiLine
                    showTitles: modelData.showTitles
                    useIcons: modelData.useIcons
                    showInstallations: modelData.showInstallations

                    onInstallationClicked: id => {
                        root.mainWindow.openInstallationDetails(id)
                    }
                    onGameClicked: id => {
                        root.mainWindow.openGameDetails(id)
                    }

                    Connections {
                        target: scrollView
                        function onContentYChanged() {
                            row.scrolled()
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
