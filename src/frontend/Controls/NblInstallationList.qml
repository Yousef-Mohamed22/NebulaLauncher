pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root
    required property int filterId
    required property bool multiLine
    required property bool showTitles
    required property bool useIcons

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

    signal installationClicked(int id)

    implicitHeight: content.height

    Item {
        id: content
        width: root.width
        height: loader.item == null ? 0 : loader.item.height

        property NblFilterModel filterModel: NblFilterModel {
            pageSize: 10
            filterId: root.filterId
        }

        property Component buttonDelegate: Component {
            NblInstallationButton {
                required property var modelData

                width: root.itemWidth
                height: root.rowHeight

                installationId: modelData.id
                fallbackSource: "qrc:/qt/qml/Nebula/images/" + (root.useIcons ? "UnknownIcon.png" : "UnknownPoster.png")
                imageSource: root.getImageSource(modelData) ?? fallbackSource
                title: modelData.title
                aspectRatio: root.useIcons ? 1 : (2/3)
                showTitle: root.showTitles
                titleAlignment: root.useIcons ? Qt.AlignHCenter : Qt.AlignLeft

                onClicked: {
                    root.installationClicked(modelData.id)
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
                        model: content.filterModel
                        delegate: content.buttonDelegate
                    }
                }

                onAtXEndChanged: {
                    if(atXEnd && !content.filterModel.isLoading) {
                        content.filterModel.loadNextPage()
                    }
                }
            }
        }
        Component {
            id: multiLineComponent

            Flow {

                function scrolled() {
                    if(triggerItem.isOnScreen()) {
                        content.filterModel.loadNextPage()
                    }
                }

                id: multiLineList

                width: parent.width
                padding: 8
                spacing: 16

                Repeater {
                    model: content.filterModel
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
            return "file://" + ApplicationContext.dataFolder + "/media/icon/" + modelData.icon
        }
        if(modelData.poster.byteLength == 0) return null
        return "file://" + ApplicationContext.dataFolder + "/media/poster/" + modelData.poster

    }
}
