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

Item {

    id: root
    required property bool multiLine
    required property bool showTitles
    required property bool showFilterInfo
    required property bool useIcons
    required property var model

    property bool allLoaded: false

    property real requestedWidth: 180

    readonly property real aspectRatio: useIcons ? 1 : (2/3)
    readonly property real itemWidth: calculateWidth(width - 16, requestedWidth)
    //readonly property real rowHeight: (itemWidth / aspectRatio) + (showTitles ? 24 : 0) + (showFilterInfo ? 24 : 0)

    function calculateWidth(rootWidth, itemWidth) {

        let idealWidth = itemWidth + 16 // Account for spacing
        let numItems = rootWidth / idealWidth
        let wholeNumItems = Math.round(numItems)

        let remainingSpace = numItems - wholeNumItems
        let spacePerItem = remainingSpace / wholeNumItems

        let realWidth = (idealWidth - 16) * (1 + spacePerItem)
        return realWidth
    }

    signal itemClicked(int id)
    signal loadMore

    implicitHeight: content.height

    Item {
        id: content
        width: root.width
        height: loader.item == null ? 0 : loader.item.height

        property Component buttonDelegate: Component {
            GameButton {
                required property var modelData

                width: root.itemWidth

                filterInfo: modelData.filterInfo ?? ""
                filterInfoIcon: modelData.filterInfoIcon ?? ""

                fallbackSource: "qrc:/qt/qml/Nebula/images/" + (root.useIcons ? "UnknownIcon.png" : "UnknownPoster.png")
                imageSource: root.getImageSource(modelData) ?? fallbackSource
                title: root.showTitles ? modelData.title : ""
                aspectRatio: root.useIcons ? 1 : (2/3)
                titleAlignment: root.useIcons ? Qt.AlignHCenter : Qt.AlignLeft

                onClicked: {
                    root.itemClicked(modelData.id)
                }
            }
        }

        Loader {
            id: loader
            width: root.width
            sourceComponent: root.multiLine ? multiLineComponent : singleLineComponent
            active: true

            onLoaded: {
                item.scrolled()
            }
        }

        Component {
            id: singleLineComponent
            Flickable {
                function scrolled() {}

                width: parent.width
                height: singleLineList.height

                contentHeight: singleLineList.height
                contentWidth: singleLineList.width
                
                clip: true
                Row {
                    id: singleLineList

                    spacing: 16
                    padding: 8

                    Repeater {
                        model: root.model
                        delegate: content.buttonDelegate
                    }
                }

                onAtXEndChanged: {
                    if(atXEnd) { 
                        root.loadMore()
                    }
                }
            }
        }
        Component {
            id: multiLineComponent

            Flow {

                function scrolled() {
                    while(!root.allLoaded && isAtEnd()) {
                        root.loadMore()
                    }
                }
                function isAtEnd() {

                    if(multiLineRepeater.count == 0) return true
                    let globalPos = multiLineRepeater.itemAt(multiLineRepeater.count - 1).mapToItem(null, 0, 0);

                    let itemLeft = globalPos.x;
                    let itemRight = globalPos.x + width;
                    let itemTop = globalPos.y;
                    let itemBottom = globalPos.y + height;

                    let insideX = (itemRight > 0) && (itemLeft < root.Window.width);
                    let insideY = (itemBottom > 0) && (itemTop < root.Window.height);

                    return insideX && insideY;
                }

                id: multiLineList

                width: parent.width
                padding: 8
                spacing: 16

                Repeater {
                    id: multiLineRepeater
                    model: root.model
                    delegate: content.buttonDelegate

                    onCountChanged: {
                        parent.scrolled()
                    }
                }
            }
        }
    }

    function scrolled() {
        loader.item.scrolled()
    }

    function getImageSource(modelData) {

        if(root.useIcons) {
            if(modelData.icon.byteLength == 0) return null
            return modelData.icon 
        }
        if(modelData.poster.byteLength == 0) return null
        return modelData.poster 

    }
}
