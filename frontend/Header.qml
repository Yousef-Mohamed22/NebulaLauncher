import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
