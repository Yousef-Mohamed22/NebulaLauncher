import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property string icon
    required property string text

    property color inactiveColor: "transparent"

    property int radius: 8
    property int leftMargin: 8

    property bool centered: false

    signal clicked

    implicitHeight: 48

    Rectangle {
        id: background
        anchors.fill: parent
        color: mouseArea.containsMouse ? Theme.colors.border : root.inactiveColor
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

        anchors.centerIn: parent

        width: root.centered ? textLabel.implicitWidth + 8 + 24 : parent.width

        anchors.leftMargin: root.leftMargin
        spacing: 8

        Label {
            Layout.preferredWidth: 24
            text: root.icon
            font.family: Theme.nerdFont
            font.pixelSize: 24
            color: Theme.colors.text
        }

        Label {
            id: textLabel
            Layout.fillWidth: true
            text: root.text
            font.family: Theme.font
            font.pixelSize: 16
            color: Theme.colors.text
        }
    }
}
