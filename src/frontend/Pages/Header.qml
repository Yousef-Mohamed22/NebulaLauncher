import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

Item {

    id: root

    property Sidebar sidebar

    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.colors.layer1
        border.width: 2
        border.color: Theme.colors.border
        radius: 8
    }

    FlexboxLayout {
        anchors.fill: background
        anchors.margins: 8

        alignItems: FlexboxLayout.AlignCenter
        justifyContent: FlexboxLayout.JustifySpaceBetween

        RowLayout {

            spacing: 8

            Rectangle {
                color: "transparent"

                implicitWidth: 48
                implicitHeight: 48

                Label {
                    text: "󰍜"
                    font.family: Theme.nerdFont
                    font.pixelSize: 28
                    color: Theme.colors.text
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.sidebar.collapsed = !root.sidebar.collapsed
                    }
                }
            }

            Item {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                NblImage {
                    id: logoSource
                    anchors.fill: parent
                    source: "qrc:/qt/qml/Nebula/images/Nebula.png"
                    usePlaceholder: false
                    layer.enabled: true
                    visible: false
                }
                ShaderEffect {
                    anchors.fill: parent
                    property variant source: logoSource
                    property real uHue: Theme.colors.accent.hsvHue * (2 * Math.PI)
                    fragmentShader: "qrc:/shaders/hueShift.frag.qsb"
                }
            }

            Label {
                text: "Nebula"
                font.family: Theme.font
                font.bold: true
                font.pixelSize: 24
                color: Theme.colors.text
            }

        }
        Rectangle {
        }
        Rectangle {
        }

    }
}
