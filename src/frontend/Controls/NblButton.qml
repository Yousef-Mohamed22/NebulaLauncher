import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property string icon
    required property string text

    signal clicked

    implicitHeight: 48

    Rectangle {
        id: background
        anchors.fill: parent
        color: mouseArea.containsMouse ? Theme.colors.border : Theme.colors.layer1
        radius: 8
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

    RowLayout {

        anchors.centerIn: parent
        spacing: 8

        Label {
            text: root.icon
            font.family: Theme.nerdFont
            font.pixelSize: 24
            color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
        }

        Label {
            text: root.text
            font.family: Theme.font
            font.pixelSize: 16
            color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
        }

    }
}
