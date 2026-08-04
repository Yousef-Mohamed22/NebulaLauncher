pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root
    required property bool multiLine
    required property bool showTitles
    required property bool useIcons
    required property var model

    property bool allLoaded: false

    property real requestedWidth: 180

    readonly property real aspectRatio: useIcons ? 1 : (2/3)
    readonly property real itemWidth: calculateWidth(width, requestedWidth)
    readonly property real rowHeight: (itemWidth / aspectRatio) + (showTitles ? 24 : 0)

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
            NblGameButton {
                required property var modelData

                width: root.itemWidth
                height: root.rowHeight

                fallbackSource: "qrc:/qt/qml/Nebula/images/" + (root.useIcons ? "UnknownIcon.png" : "UnknownPoster.png")
                imageSource: root.getImageSource(modelData) ?? fallbackSource
                title: modelData.title
                aspectRatio: root.useIcons ? 1 : (2/3)
                showTitle: root.showTitles
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
        }

        Component {
            id: singleLineComponent
            Flickable {
                function scrolled() {}

                width: parent.width
                height: root.rowHeight + 16

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

                Component.onCompleted: {
                    while(!root.allLoaded && triggerItem.isOnScreen()) {
                        root.loadMore()
                    }
                }

                function scrolled() {
                    while(!root.allLoaded && triggerItem.isOnScreen()) {
                        root.loadMore()
                    }
                }

                id: multiLineList

                width: parent.width
                padding: 8
                spacing: 16

                Repeater {
                    model: root.model
                    delegate: content.buttonDelegate
                }

                Item {
                    id: triggerItem
                    function isOnScreen() {

                        let globalPos = mapToItem(null, 0, 0);

                        let itemLeft = globalPos.x;
                        let itemRight = globalPos.x + width;
                        let itemTop = globalPos.y;
                        let itemBottom = globalPos.y + height;

                        let insideX = (itemRight > 0) && (itemLeft < root.Window.width);
                        let insideY = (itemBottom > 0) && (itemTop < root.Window.height);

                        return insideX && insideY;
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
            return ApplicationContext.mediaUrl(modelData.icon, "icon") //"file://" + ApplicationContext.dataFolder + "/media/icon/" + modelData.icon
        }
        if(modelData.poster.byteLength == 0) return null
        return ApplicationContext.mediaUrl(modelData.poster, "poster") //"file://" + ApplicationContext.dataFolder + "/media/poster/" + modelData.poster

    }
}
