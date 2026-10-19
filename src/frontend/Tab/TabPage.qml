// Copyright 2026 jwall64
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

pragma ComponentBehavior: Bound

import QtQuick

import Nebula

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
                model: TabFilterModel {
                    id: tabModel
                    pageSize: 20
                    tabId: root.tabId
                }
                delegate: FilterRow {

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
                    showFilterInfo: modelData.showFilterInfo
                    iconIsImage: modelData.iconIsImage

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
