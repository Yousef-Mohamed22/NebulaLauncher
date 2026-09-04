pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtQuick.Shapes
import QtQuick.Window

Item {

    id: root

    required property string imageSource
    property bool highlighted

    property alias sourceSize: image.sourceSize
    property alias fillMode: image.fillMode

    signal clicked


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

        width: root.width
        height: root.height
        anchors.horizontalCenter: parent.horizontalCenter

        NblImage {
            id: image
            anchors.fill: parent
            source: root.imageSource
            visible: false
            layer.enabled: true
            fillMode: Image.PreserveAspectCrop

            property bool loadFailed: false
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
        }
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
