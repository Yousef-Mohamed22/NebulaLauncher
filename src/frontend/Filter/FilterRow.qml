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

import QtQuick
import QtQuick.Controls

import Nebula

Item {

    id: root

    required property int filterId
    required property string name
    required property string icon
    required property bool colorizeIcon
    required property bool multiLine
    required property bool useIcons
    required property bool showTitles
    required property bool showInstallations
    required property bool showFilterInfo
    required property bool iconIsImage

    property bool showRowTitle: true

    implicitHeight: content.height

    signal itemClicked(int id)
    signal gameClicked(int id)
    signal installationClicked(int id)

    Column {

        id: content
        width: parent.width

        Row {

            x: 8
            height: 32
            spacing: 8

            visible: root.showRowTitle

            NblInlineIcon {
                width: 20
                height: 20
                anchors.verticalCenter: parent.verticalCenter
                icon: root.icon
                isImage: root.iconIsImage
                color: Theme.colors.text
                colorize: root.colorizeIcon
            }
            Label {
                text: root.name
                font.family: Theme.font
                font.pixelSize: 20
                font.bold: true
                color: Theme.colors.text
            }
        }

        GameList {

            id: list
            width: content.width

            multiLine: root.multiLine
            useIcons: root.useIcons
            showTitles: root.showTitles
            showFilterInfo: root.showFilterInfo

            model: FilterModel {
                pageSize: 30
                filterId: root.filterId
                searchInstallations: root.showInstallations
            }
            onLoadMore: {
                model.loadNextPage()
                allLoaded = model.allLoaded
            }

            onItemClicked: id => {
                root.itemClicked(id)
                if(root.showInstallations) {
                    root.installationClicked(id)
                } else {
                    root.gameClicked(id)
                }
            }
        }
        
    }

    function scrolled() {
        list.scrolled()
    }

}
