pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {

    id: root
    required property NblFilterRef filter
    required property MainWindow mainWindow

    implicitHeight: content.height
    width: parent.width

    Item {

        id: content
        height: loader.item != null ? loader.item.height : 0

        Loader {
            id: loader
            sourceComponent: root.filter.multiLine ? multiLineComponent : singleLineComponent
            active: true
        }

        Component {
            id: singleLineComponent
            Flickable {

                id: flickable

                height: singleLineList.height + 8
                width: root.width

                contentWidth: singleLineList.width
                contentHeight: singleLineList.height + 8

                clip: true
                flickableDirection: Flickable.HorizontalFlick

                Row {
                    id: singleLineList
                    spacing: 8
                    Repeater {
                        model: root.filter.installations
                        NblPosterImage {
                            required property NblInstallationRef modelData
                            installation: modelData
                            mainWindow: root.mainWindow
                        }
                    }
                }

                ScrollBar.horizontal: ScrollBar {
                    parent: flickable
                    anchors.bottom: flickable.bottom
                    anchors.left: flickable.left
                    anchors.right: flickable.right
                    active: flickable.movingHorizontally
                }
                ScrollBar.vertical: null
            }
        }

        Component {
            id: multiLineComponent
            Item {
                height: multiLineList.height + 8
                Flow {
                    id: multiLineList
                    width: root.width
                    spacing: 8
                    Repeater {
                        model: root.filter.installations
                        NblPosterImage {
                            required property NblInstallationRef modelData
                            installation: modelData
                            mainWindow: root.mainWindow
                        }
                    }   
                }
            }
        }
    }
}
