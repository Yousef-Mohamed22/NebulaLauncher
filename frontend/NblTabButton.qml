import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property MainWindow mainWindow
    property NblTabRef ref

    property bool collapsed: false
    property bool hovered: false

    height: 48

    Rectangle {
        id: background
        anchors.fill: parent
        color: hovered ? Theme.colors.border : Theme.colors.layer0
        radius: 8
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onEntered: root.hovered = true
        onExited: root.hovered = false
        onClicked: {
            mainWindow.openTab(ref)
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Rectangle {
            color: "transparent"
            width: 32
            height: 32

            Label {
                text: ref.icon
                anchors.centerIn: parent
                font.family: Theme.nerdFont
                font.pixelSize: 24
                color: Theme.colors.text
            }
        }

        Rectangle {
            color: "transparent"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter
            visible: !collapsed
            
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: ref.name
                font.family: Theme.font
                font.pixelSize: 16
                color: Theme.colors.text
            }
        }

    }

}
