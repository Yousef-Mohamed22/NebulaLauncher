pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtQuick.Controls
import QtQuick.Shapes
import QtQuick.Window

Item {

    id: root

    required property string imageSource
    required property string title
    required property real aspectRatio
    required property bool showTitle
    property string fallbackSource
    property bool doScaling: true
    property var titleAlignment: Qt.AlignLeft

    signal clicked

    readonly property real scaleFactor: Math.min(width / aspectRatio, height - (showTitle ? 48 : 0))
    property bool highlighted

    Item {

        id: highlightShape
        visible: root.highlighted

        width: parent.width + 12
        height: parent.height + 12
        x: -6
        y: -6

        Shape {
            anchors.fill: parent        
            preferredRendererType: Shape.CurveRenderer

            ShapePath {
                PathRectangle {
                    width: highlightShape.width
                    height: highlightShape.height
                    radius: 14
                }

                strokeColor: "transparent"
                strokeWidth: -1 

                fillGradient: ConicalGradient {

                    centerX: root.width / 2
                    centerY: root.height / 2
                    angle: 0
                    NumberAnimation on angle {
                        from: 0
                        to: 360
                        duration: 2000
                        loops: Animation.Infinite
                        running: true
                    }

                    GradientStop { position: 0.0; color: Theme.colors.accent.darker(1.3) }
                    GradientStop { position: 0.5; color: Theme.colors.accent.lighter(1.3) }
                    GradientStop { position: 1.0; color: Theme.colors.accent.darker(1.3) }
                }
            }
        }
        Rectangle {

            anchors.fill: parent
            anchors.margins: 4
            radius: 10
            color: Theme.colors.layer0
        }
    }


    Item {

        width: root.aspectRatio * root.scaleFactor
        height: root.scaleFactor
        anchors.horizontalCenter: parent.horizontalCenter

        NblImage {
            id: image
            anchors.fill: parent
            source: root.imageSource
            visible: false
            layer.enabled: true

            property bool loadFailed: false

            onStatusChanged: {
                if (status === Image.Error) {
                    if (!loadFailed) {
                        loadFailed = true
                        source = ""
                        source = root.fallbackSource
                    }
                } else if (status === Image.Ready) {
                    loadFailed = false
                }
            }
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
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0

            scale: root.doScaling && mouseArea.containsMouse ? 1.05 : 1.0
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
