pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

Item {

    id: root

    required property int installationId
    required property string imageSource
    required property string title
    required property real aspectRatio
    required property bool showTitle
    property var titleAlignment: Qt.AlignLeft

    signal clicked

    property real scaleFactor: Math.min(width / aspectRatio, height - (showTitle ? 48 : 0))


    Item {

        width: root.aspectRatio * root.scaleFactor
        height: root.scaleFactor
        anchors.horizontalCenter: parent.horizontalCenter

        Image {
            id: image
            anchors.fill: parent
            source: root.imageSource
            visible: false
            layer.enabled: true
        }

        Item {
            id: maskShape
            anchors.fill: image
            visible: false
            layer.enabled: true
            Rectangle {
                anchors.fill: parent
                radius: 8
            }
        }

        MultiEffect {
            source: image
            anchors.fill: image
            maskEnabled: true
            maskSource: maskShape

            scale: mouseArea.containsMouse ? 1.05 : 1.0
            Behavior on scale {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    Label {

        y: image.height + 8
        x: 8
        width: parent.width - 16

        visible: root.showTitle
        text: root.title
        elide: Text.ElideRight
        horizontalAlignment: root.titleAlignment

        font.family: Theme.font
        font.weight: 500
        font.pixelSize: 16
        color: Theme.colors.text
    }


    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.clicked()
        }
    }
}
