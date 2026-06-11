import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property MainWindow mainWindow
    default property alias content: contentContainer.children

    property string applyText: "Apply"
    property string okText: "OK"
    property string cancelText: "Cancel"

    width: parent.width
    height: parent.height

    signal apply
    signal cancel
    signal close

    Rectangle {
        anchors.fill: parent
        color: "#AA000000"
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        hoverEnabled: true
    }

    Item {
        width: Math.min(parent.width - 16, 800)
        height: Math.min(parent.height - 16, 600)

        anchors.centerIn: parent

        Rectangle {
            anchors.fill: parent
            color: Theme.colors.layer0
            radius: 16
        }

        ColumnLayout {

            anchors.fill: parent
            anchors.margins: 16
            spacing: 8

            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 16
                id: flickable

                contentWidth: width - 48
                contentHeight: contentContainer.height
                flickableDirection: Flickable.VerticalFlick
                clip: true

                Column {

                    id: contentContainer
                    width: flickable.contentWidth
                    x: 24

                    spacing: 8

                    
                }

                ScrollBar.horizontal: null
                ScrollBar.vertical: ScrollBar {
                    parent: flickable
                    anchors.top: flickable.top
                    anchors.bottom: flickable.bottom
                    anchors.right: flickable.right
                    active: flickable.movingVertically
                }

            }

            Item {

                Layout.fillWidth: true
                Layout.preferredHeight: 40

                Row {

                    anchors.fill: parent

                    layoutDirection: Qt.RightToLeft
                    spacing: 8

                    Item {

                        id: okBtn
                        width: 128
                        height: parent.height

                        Rectangle {
                            anchors.fill: parent
                            color: Theme.colors.accent
                            radius: 8
                        }

                        Label {
                            anchors.centerIn: parent
                            font.pixelSize: 16
                            font.weight: 500
                            color: Theme.colors.text
                            text: root.okText
                        }

                        MouseArea {
                            id: addMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.apply()
                                root.close()
                            }
                        }
                    }

                    Item {

                        id: applyBtn
                        width: 128
                        height: parent.height

                        Rectangle {
                            anchors.fill: parent
                            color: okMouseArea.containsMouse ? Theme.colors.border : Theme.colors.layer1
                            radius: 8
                        }

                        Label {
                            anchors.centerIn: parent
                            font.pixelSize: 16
                            font.weight: 500
                            color: Theme.colors.text
                            text: root.applyText
                        }

                        MouseArea {
                            id: okMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.apply()
                            }
                        }
                    }

                    Item {

                        id: cancelBtn
                        width: 128
                        height: parent.height

                        Rectangle {
                            anchors.fill: parent
                            color: cancelMouseArea.containsMouse ? Theme.colors.border : Theme.colors.layer1
                            radius: 8
                        }

                        Label {
                            anchors.centerIn: parent
                            font.pixelSize: 16
                            font.weight: 500
                            color: Theme.colors.text
                            text: root.cancelText
                        }

                        MouseArea {
                            id: cancelMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.cancel()
                                root.close()
                            }
                        }
                    }
                }
            }
        }
    }
}
