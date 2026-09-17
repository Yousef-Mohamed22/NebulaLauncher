pragma ComponentBehavior: Bound

import QtQuick

import Nebula.Controls

Item {

    id: root

    signal installationClicked(int installationId)
    signal gameClicked(int gameId)

    required property int tabId

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
                    pageSize: 20
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
                        root.installationClicked(id)
                    }
                    onGameClicked: id => {
                        root.gameClicked(id)
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
