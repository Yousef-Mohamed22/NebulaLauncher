pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root
    required property int filterId
    required property bool multiLine
    required property bool showTitles
    required property bool useIcons
    
    required property MainWindow mainWindow


    property real rowHeight: (useIcons ? 180 : 270) + (showTitles ? 24 : 0)

    implicitHeight: content.height
    width: parent.width

    Item {

        anchors.fill: root
        id: content
        height: loader.item != null ? loader.item.height : 0

        Loader {

            anchors.fill: parent
            id: loader
            sourceComponent: root.multiLine ? multiLineComponent : singleLineComponent
            active: true
        }

        Component {
            id: singleLineComponent

            ListView {
                id: singleLineList

                x: 8
                spacing: 16

                height: root.rowHeight
                width: root.width

                orientation: ListView.Horizontal

                model: NblFilterModel {
                    id: filterModel
                    pageSize: 10
                    filterId: root.filterId
                }

                delegate: NblInstallationButton {
                    required property var modelData

                    width: 180
                    height: singleLineList.height

                    installationId: modelData.id
                    imageSource: root.getImageSource(modelData)
                    title: modelData.title
                    aspectRatio: root.useIcons ? 1 : (2/3)
                    showTitle: root.showTitles
                    titleAlignment: root.useIcons ? Qt.AlignHCenter : Qt.AlignLeft

                    onClicked: {
                        root.mainWindow.openInstallationDetails(modelData.id)
                    }
                }

                onAtXEndChanged: {
                    if(atXEnd && !filterModel.isLoading) {
                        filterModel.loadNextPage()
                    }
                }
            }
        }

        Component {
            id: multiLineComponent

            GridView {
                id: multiLineList
                width: root.width
                height: contentHeight
                interactive: false

                cellWidth: 180 + 16
                cellHeight: root.rowHeight + 16

                model: NblFilterModel {
                    id: filterModel
                    pageSize: 10
                    filterId: root.filterId
                }

                delegate: Item {
                    required property var modelData

                    width: multiLineList.cellWidth
                    height: multiLineList.cellHeight

                    NblInstallationButton {

                        anchors.fill: parent
                        anchors.margins: 8

                        installationId: parent.modelData.id
                        imageSource: root.getImageSource(parent.modelData)
                        title: parent.modelData.title
                        aspectRatio: root.useIcons ? 1 : (2/3)
                        showTitle: root.showTitles
                        titleAlignment: root.useIcons ? Qt.AlignHCenter : Qt.AlignLeft

                        onClicked: {
                            root.mainWindow.openInstallationDetails(parent.modelData.id)
                        }
                    }
                }

                onAtYEndChanged: {
                    if(atYEnd && !filterModel.isLoading) {
                        filterModel.loadNextPage()
                    }
                }

            }
        }
    }

    function getImageSource(modelData) {

        if(root.useIcons) return "file://" + ApplicationContext.dataFolder + "/media/icon/" + modelData.icon
        return "file://" + ApplicationContext.dataFolder + "/media/poster/" + modelData.poster

    }
}
