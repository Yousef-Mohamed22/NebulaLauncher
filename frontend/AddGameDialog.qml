import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property MainWindow mainWindow

    width: parent.width
    height: parent.height

    Rectangle {
        anchors.fill: parent
        color: "#AA000000"
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
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

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 24
                id: content

                Column {

                    spacing: 8
                    anchors.fill: parent

                    NblComboBox {
                        id: gameBox
                        label: "Game"
                        width: parent.width

                        model: [ "New Game" ]
                    }

                    NblTextBox {
                        id: titleField
                        label: "Title"
                        width: parent.width
                    }

                    NblTextBox {
                        id: commandField
                        label: "Command"
                        width: parent.width
                    }

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

                        id: addBtn
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
                            text: "Add"
                        }

                        MouseArea {
                            id: addMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                console.log("clicked add game")
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
                            text: "Cancel"
                        }

                        MouseArea {
                            id: cancelMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.close()
                            }
                        }

                    }

                }

            }
        }
    }

    function close() {

        root.mainWindow.cancelAddGame()
    }
}
