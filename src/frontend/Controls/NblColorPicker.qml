import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Effects

Item {

    id: root

    property color color

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        RowLayout {

            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                Canvas {

                    id: gridCanvas
                    anchors.fill: parent

                    visible: false
                    layer.enabled: true

                    onPaint: {

                        let ctx = getContext("2d")
                        ctx.reset()

                        let imageData = ctx.getImageData(0, 0, width, height)

                        for(let y = 0 ; y < imageData.height ; y++) {
                            for(let x = 0 ; x < imageData.width ; x++) {
                                let sat = (x / imageData.width) 
                                let val = (y / imageData.height)

                                let color = Qt.hsva(0.0, sat, val, 1.0)
                                let offset = (x + (y * imageData.width)) * 4

                                imageData.data[offset] = color.r * 255
                                imageData.data[offset + 1] = color.g * 255
                                imageData.data[offset + 2] = color.b * 255
                                imageData.data[offset + 3] = 255
                            }
                        }

                        ctx.drawImage(imageData, 0, 0)
                    }
                }

                ShaderEffect {
                    anchors.fill: parent
                    property variant source: gridCanvas
                    property real uHue: root.color.hsvHue * (2 * Math.PI)
                    fragmentShader: "qrc:/shaders/hueShift.frag.qsb"
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: ev => {
                        root.color = Qt.hsva(
                            root.color.hsvHue, 
                            Math.min(width, Math.max(ev.x, 0)) / width,
                            Math.min(height, Math.max(ev.y, 0)) / height,
                            1.0)
                    }
                    onPositionChanged: ev => {
                        if(pressed) {
                            root.color = Qt.hsva(
                                root.color.hsvHue, 
                                Math.min(width, Math.max(ev.x, 0)) / width,
                                Math.min(height, Math.max(ev.y, 0)) / height,
                                1.0)
                        }
                    }
                }

                Shape {
                    width: 12
                    height: 12
                    preferredRendererType: Shape.CurveRenderer

                    x: (root.color.hsvSaturation * parent.width) - (width / 2)
                    y: (root.color.hsvValue * parent.height) - (height / 2)

                    ShapePath {
                        strokeColor: Theme.colors.text
                        strokeWidth: 4
                        fillColor: "transparent"
                        startX: 6 // width / 2
                        startY: 2 // (strokeWidth / 2)

                        PathArc {
                            x: 6 // startX
                            y: 10 // width - startY
                            radiusX: 4 // (width / 2) - (strokeWidth / 2)
                            radiusY: 4
                            useLargeArc: true
                        }

                        PathArc {
                            x: 6
                            y: 2
                            radiusX: 4
                            radiusY: 4
                            useLargeArc: true
                        }
                    }
                }

            }
            Item { // Hue slider
                Layout.fillHeight: true
                Layout.preferredWidth: 32

                Canvas {
                    id: hueCanvas
                    width: parent.width - 12
                    height: parent.height
                    anchors.left: parent.left

                    onPaint: {
                        let ctx = getContext("2d")
                        ctx.reset()

                        let imageData = ctx.getImageData(0, 0, width, height)

                        for(let y = 0 ; y < imageData.height ; y++) {
                            
                            let hue = (y / imageData.height) 
                            let color = Qt.hsva(hue, 1.0, 1.0, 1.0)

                            for(let x = 0 ; x < imageData.width ; x++) {
                                let offset = (x + (y * imageData.width)) * 4

                                imageData.data[offset] = color.r * 255
                                imageData.data[offset + 1] = color.g * 255
                                imageData.data[offset + 2] = color.b * 255
                                imageData.data[offset + 3] = 255
                            }
                        }

                        ctx.drawImage(imageData, 0, 0)
                    }
                }

                MouseArea {
                    anchors.fill: hueCanvas
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onPositionChanged: ev => {
                        if(pressed) {
                            root.color.hsvHue = Math.min(height, Math.max(ev.y, 0)) / height
                            //canvas.requestPaint()
                        }
                    }
                    onClicked: ev => {
                        root.color.hsvHue = Math.min(height, Math.max(ev.y, 0)) / height
                        //canvas.requestPaint()
                    }
                }

                Canvas {
                    y: root.hue * parent.height - (height / 2)
                    anchors.right: parent.right
                    width: 10
                    height: 8

                    onPaint: {
                        let ctx = getContext("2d")
                        ctx.reset()

                        ctx.beginPath()
                        ctx.moveTo(0, height / 2)
                        ctx.lineTo(width, 0)
                        ctx.lineTo(width, height)
                        ctx.closePath()

                        ctx.fillStyle = Theme.colors.text; 
                        ctx.fill();
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 32

            color: root.color
        }
    }

}
