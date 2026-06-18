pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root
    required property int filterId
    required property bool multiLine
    required property bool showTitles
    required property bool useIcons
    
    property real rowHeight: (useIcons ? 180 : 270) + (showTitles ? 24 : 0)

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

                width: 180
                height: root.rowHeight

                installationId: modelData.id
                imageSource: root.getImageSource(modelData)
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


    // Item {
    //
    //     anchors.fill: root
    //     id: content
    //     height: loader.item != null ? loader.item.height : 0
    //
    //     Loader {
    //
    //         anchors.fill: parent
    //         id: loader
    //         sourceComponent: root.multiLine ? multiLineComponent : singleLineComponent
    //         active: true
    //     }
    //
    //     Component {
    //         id: singleLineComponent
    //
    //         ListView {
    //             id: singleLineList
    //
    //             spacing: 16
    //             anchors.margins: 8
    //
    //             height: root.rowHeight
    //             width: root.width
    //             clip: true
    //
    //             orientation: ListView.Horizontal
    //
    //             model: NblFilterModel {
    //                 id: filterModel
    //                 pageSize: 10
    //                 filterId: root.filterId
    //             }
    //
    //             delegate: NblInstallationButton {
    //                 required property var modelData
    //
    //                 width: 180
    //                 height: singleLineList.height
    //
    //                 installationId: modelData.id
    //                 imageSource: root.getImageSource(modelData)
    //                 title: modelData.title
    //                 aspectRatio: root.useIcons ? 1 : (2/3)
    //                 showTitle: root.showTitles
    //                 titleAlignment: root.useIcons ? Qt.AlignHCenter : Qt.AlignLeft
    //
    //                 onClicked: {
    //                     root.mainWindow.openInstallationDetails(modelData.id)
    //                 }
    //             }
    //
    //             onAtXEndChanged: {
    //                 if(atXEnd && !filterModel.isLoading) {
    //                     filterModel.loadNextPage()
    //                 }
    //             }
    //         }
    //     }
    //
    //     Component {
    //         id: multiLineComponent
    //
    //         GridView {
    //             id: multiLineList
    //             width: root.width
    //             height: contentHeight
    //             interactive: false
    //
    //             cellWidth: 180 + 16
    //             cellHeight: root.rowHeight + 16
    //
    //             model: NblFilterModel {
    //                 id: filterModel
    //                 pageSize: 10
    //                 filterId: root.filterId
    //             }
    //
    //             delegate: Item {
    //                 required property var modelData
    //
    //                 width: multiLineList.cellWidth
    //                 height: multiLineList.cellHeight
    //
    //                 NblInstallationButton {
    //
    //                     anchors.fill: parent
    //                     anchors.margins: 8
    //
    //                     installationId: parent.modelData.id
    //                     imageSource: root.getImageSource(parent.modelData)
    //                     title: parent.modelData.title
    //                     aspectRatio: root.useIcons ? 1 : (2/3)
    //                     showTitle: root.showTitles
    //                     titleAlignment: root.useIcons ? Qt.AlignHCenter : Qt.AlignLeft
    //
    //                     onClicked: {
    //                         root.mainWindow.openInstallationDetails(parent.modelData.id)
    //                     }
    //                 }
    //             }
    //
    //             onAtYEndChanged: {
    //                 if(atYEnd && !filterModel.isLoading) {
    //                     filterModel.loadNextPage()
    //                 }
    //             }
    //
    //         }
    //     }
    //}


    function scrolled() {
        loader.item.scrolled()
    }

    function getImageSource(modelData) {

        if(root.useIcons) return "file://" + ApplicationContext.dataFolder + "/media/icon/" + modelData.icon
        return "file://" + ApplicationContext.dataFolder + "/media/poster/" + modelData.poster

    }
}
