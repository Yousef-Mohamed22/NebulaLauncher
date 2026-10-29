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
import QtQuick.Controls

import Nebula

Item {

    id: root

    required property int tag
    required property string name
    required property color color

    property bool useIcons: false
    property bool showTitles: false

    signal gameClicked(int gameId)

    ScrollView {

        id: scrollView
        width: parent.width
        height: parent.height - 8
        y: 8

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        contentHeight: list.height

        Flickable {

            anchors.fill: parent
            interactive: true
            clip: true

            id: flickable

            onContentYChanged: list.scrolled()

            Column {

                id: content
                width: parent.width

                Row {
                    x: 8
                    height: 32
                    spacing: 8

                    NblInlineIcon {
                        width: 20
                        height: 20
                        anchors.verticalCenter: parent.verticalCenter
                        icon: ""
                        color: Theme.colors.text
                    }

                    Label {
                        text: "Games tagged" 
                        font.family: Theme.font
                        font.pixelSize: 20
                        font.bold: true
                        color: Theme.colors.text
                    }

                    Tag {
                        enabled: false
                        text: root.name
                        color: root.color
                        height: 28
                        textSize: 16
                        width: Math.max(implicitWidth, 98)
                    }
                }

                GameList {

                    id: list
                    width: parent.width
                    viewportHeight: scrollView.height

                    multiLine: true
                    useIcons: root.useIcons
                    showTitles: root.showTitles
                    showFilterInfo: false

                    model: GameWithTagModel {
                        pageSize: list.pageSize
                        tag: root.tag
                    }
                    onLoadMore: {
                        model.loadNextPage()
                        allLoaded = model.allLoaded
                    }

                    onItemClicked: id => {
                        root.gameClicked(id)
                    }
                }
            }
        }
    }

    function refresh() { }
}
