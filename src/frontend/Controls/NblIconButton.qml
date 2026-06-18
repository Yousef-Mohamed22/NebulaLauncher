import QtQuick
import QtQuick.Controls

Item {

    id: root

    required property string icon

    signal clicked

    height: 48
    width: 48

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

    Label {
        text: root.icon
        anchors.centerIn: parent
        font.family: Theme.nerdFont
        font.pixelSize: 24
        color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
    }

}
