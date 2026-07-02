import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property MainWindow mainWindow
    default property alias content: contentContainer.children

    property list<QtObject> above: []
    property list<QtObject> navigation: []

    property string applyText: "Apply"
    property string okText: "OK"
    property string cancelText: "Cancel"

    property bool showApplyButton: true
    property bool showOkButton: true
    property bool showCancelButton: true

    property color okButtonColor: Theme.colors.accent

    property int targetWidth: 800
    property int targetHeight: 600

    width: parent.width
    height: parent.height

    signal apply
    signal cancel
    signal close

    Rectangle {
        anchors.fill: parent
        color: "#AA000000"
    }

    WheelHandler {
        blocking: true 
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        hoverEnabled: true
        onWheel: wheel => { wheel.accepted = true; }
        onPressed: mouse => { mouse.accepted = true; }
        onReleased: mouse => { mouse.accepted = true; }
    }

    Item {
        width: Math.min(parent.width - 16, root.targetWidth)
        height: Math.min(parent.height - 16, root.targetHeight)

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

            Item {
                id: contentContainer
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Item {

                Layout.fillWidth: true
                Layout.preferredHeight: 40

                RowLayout {

                    anchors.fill: parent

                    layoutDirection: Qt.RightToLeft
                    spacing: 8

                    Item {

                        id: okBtn
                        visible: root.showOkButton

                        Layout.preferredWidth: 128
                        Layout.fillHeight: true

                        Rectangle {
                            anchors.fill: parent
                            color: root.okButtonColor
                            radius: 8
                        }

                        Label {
                            anchors.centerIn: parent
                            font.pixelSize: 16
                            font.weight: 500
                            color: Theme.colors.blackOrWhite(root.okButtonColor)
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
                        visible: root.showApplyButton

                        Layout.preferredWidth: 128
                        Layout.fillHeight: true

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
                        visible: root.showCancelButton

                        Layout.preferredWidth: 128
                        Layout.fillHeight: true

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

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        children: root.navigation
                    }
                }
            }
        }
    }

    Item {
        anchors.fill: parent
        children: root.above
    }

}
