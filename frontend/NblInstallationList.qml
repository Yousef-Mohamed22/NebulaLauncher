pragma ComponentBehavior: Bound

import QtQuick

Item {

    id: root
    required property int filterId
    required property bool multiLine
    required property MainWindow mainWindow

    implicitHeight: content.height
    width: parent.width

    Item {

        id: content
        height: loader.item != null ? loader.item.height : 0

        Loader {
            id: loader
            sourceComponent: root.multiLine ? multiLineComponent : singleLineComponent
            active: true
        }

        Component {
            id: singleLineComponent

            ListView {
                id: singleLineList

                height: 270 + 16 + 8
                width: root.width

                orientation: ListView.Horizontal

                model: NblFilterModel {
                    id: filterModel
                    pageSize: 10
                    filterId: root.filterId
                }

                delegate: NblPosterImage {
                    required property var model
                    installationId: model.id
                    poster: model.poster
                    mainWindow: root.mainWindow
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
                cellHeight: 270 + 16

                model: NblFilterModel {
                    id: filterModel
                    pageSize: 10
                    filterId: root.filterId
                }

                delegate: NblPosterImage {
                    required property var model
                    installationId: model.id
                    poster: model.poster
                    mainWindow: root.mainWindow
                }

                onAtYEndChanged: {
                    if(atYEnd && !filterModel.isLoading) {
                        filterModel.loadNextPage()
                    }
                }

            }
        }
    }
}
