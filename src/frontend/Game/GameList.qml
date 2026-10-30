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

    readonly property bool allLoaded: model ? model.allLoaded : false

    property real requestedWidth: 180
    property real viewportHeight: 0

    readonly property real aspectRatio: useIcons ? 1 : (2/3)
    readonly property real itemWidth: calculateWidth(width - 16, requestedWidth)
    readonly property int columns: calculateColumns(width - 16, requestedWidth)
    readonly property real rowHeight: (itemWidth / aspectRatio) + (showTitles ? 24 : 0) + (showFilterInfo ? 24 : 0)
    readonly property real effectiveViewportHeight: viewportHeight > 0 ? viewportHeight : (root.Window ? root.Window.height : 0)

    readonly property int loadedCount: model ? model.count : 0
    readonly property int fullRowItemCount: columns > 0 ? Math.floor(loadedCount / columns) * columns : loadedCount
    readonly property int visibleItemCount: (multiLine && !allLoaded) ? fullRowItemCount : loadedCount

    readonly property int pageSize: {

        let pageWidth = width > 16 ? width : (root.Window ? root.Window.width : 0)
        if(pageWidth <= 16) return multiLine ? 12 : 6

        let columnsForPage = calculateColumns(pageWidth - 16, requestedWidth)

        if(!multiLine) {
            return columnsForPage + 1
        }

        let itemWidthForPage = calculateWidth(pageWidth - 16, requestedWidth)
        let rowHeightForPage = (itemWidthForPage / aspectRatio) + (showTitles ? 24 : 0) + (showFilterInfo ? 24 : 0)

        if(effectiveViewportHeight <= 0 || rowHeightForPage <= 0) {
            return columnsForPage * 3
        }

        let rows = Math.max(1, Math.floor(effectiveViewportHeight / rowHeightForPage) + 1)
        return Math.max(1, columnsForPage * rows)
    }

    function calculateColumns(rootWidth, itemWidth) {
        return Math.max(1, Math.round(rootWidth / (itemWidth + 16)))
    }

    function calculateWidth(rootWidth, itemWidth) {

        let idealWidth = itemWidth + 16
        let numItems = Math.max(1, rootWidth / idealWidth)
        let wholeNumItems = Math.max(1, Math.round(numItems))

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
                required property int index

                width: root.itemWidth
                visible: index < root.visibleItemCount

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
                Qt.callLater(() => {
                    if(item != null) item.scrolled()
                })
            }
        }

        Component {
            id: singleLineComponent
            Flickable {
                id: singleLineFlickable

                width: parent.width
                height: singleLineList.height

                contentHeight: singleLineList.height
                contentWidth: singleLineList.width

                clip: true

                function scrolled() {
                    if(root.allLoaded) return
                    if(width <= 0) return

                    if(singleLineRepeater.count == 0) {
                        root.loadMore()
                        return
                    }

                    if(singleLineList.width <= 0) return

                    const hasOverflow = contentWidth > width
                    const atEnd = contentX + width >= contentWidth
                    if(!hasOverflow || atEnd) {
                        root.loadMore()
                    }
                }

                onContentWidthChanged: Qt.callLater(() => scrolled())
                onWidthChanged: Qt.callLater(() => scrolled())
                onAtXEndChanged: scrolled()

                Row {
                    id: singleLineList

                    spacing: 16
                    padding: 8

                    Repeater {
                        id: singleLineRepeater
                        model: root.model
                        delegate: content.buttonDelegate
                    }
                }
            }
        }
        Component {
            id: multiLineComponent

            Flow {

                id: multiLineList

                width: parent.width
                padding: 8
                spacing: 16

                onHeightChanged: scrolled()

                function scrolled() {
                    if(root.allLoaded) return
                    if(multiLineRepeater.count == 0 || isAtEnd()) {
                        root.loadMore()
                    }
                }

                function isAtEnd() {

                    if(root.visibleItemCount <= 0) return true

                    let last = multiLineRepeater.itemAt(root.visibleItemCount - 1)
                    if(last == null) return false

                    let globalPos = last.mapToItem(null, 0, 0);

                    let itemLeft = globalPos.x;
                    let itemRight = globalPos.x + last.width;
                    let itemTop = globalPos.y;
                    let itemBottom = globalPos.y + last.height;

                    let insideX = (itemRight > 0) && (itemLeft < root.Window.width);
                    let insideY = (itemBottom > 0) && (itemTop < root.Window.height - root.rowHeight);

                    return insideX && insideY;
                }

                Repeater {
                    id: multiLineRepeater
                    model: root.model
                    delegate: content.buttonDelegate
                }
            }
        }
    }

    function scrolled() {
        if(loader.item != null) {
            loader.item.scrolled()
        }
    }

    Component.onCompleted: scrolled()

    function getImageSource(modelData) {

        if(root.useIcons) {
            if(modelData.icon.byteLength == 0) return null
            return modelData.icon 
        }
        if(modelData.poster.byteLength == 0) return null
        return modelData.poster 

    }
}
