import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property MainWindow mainWindow
    required property NblTabRef ref

    property bool collapsed: false

    height: 48

    Rectangle {
        id: background
        anchors.fill: parent
        color: mouseArea.containsMouse ? Theme.colors.border : Theme.colors.layer0
        radius: 8
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.mainWindow.openTab(root.ref)
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Rectangle {
            color: "transparent"
            implicitWidth: 32
            implicitHeight: 32

            Label {
                text: root.ref.icon
                anchors.centerIn: parent
                font.family: Theme.nerdFont
                font.pixelSize: 24
                color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
            }
        }

        Rectangle {
            color: "transparent"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter
            visible: !root.collapsed
            
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: root.ref.name
                font.family: Theme.font
                font.pixelSize: 16
                font.bold: true
                color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
            }
        }

    }

}
