import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property string icon
    required property string text

    property int radius: 8

    signal clicked

    implicitHeight: 48

    Rectangle {
        id: background
        anchors.fill: parent
        color: mouseArea.containsMouse ? Theme.colors.border : "transparent"
        radius: root.radius
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

        anchors.fill: parent
        anchors.leftMargin: 8
        spacing: 8

        Label {
            Layout.preferredWidth: 24
            text: root.icon
            font.family: Theme.nerdFont
            font.pixelSize: 24
            color: Theme.colors.text
        }

        Label {
            Layout.fillWidth: true
            text: root.text
            font.family: Theme.font
            font.pixelSize: 16
            color: Theme.colors.text
        }
    }
}
