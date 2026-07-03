import QtQuick


Item {

    id: root

    required property string text
    required property color color

    implicitWidth: 98
    implicitHeight: 24

    Rectangle {
        anchors.fill: parent
        color: root.color
        radius: root.height / 2
    }

    Text {

        text: root.text
        width: root.width - 8
        elide: Qt.ElideRight
        horizontalAlignment: Qt.AlignHCenter

        anchors.centerIn: parent
        font.pixelSize: 14
        font.family: Theme.font
        font.weight: 600
        color: Theme.colors.readableOn(root.color)

    }

}
