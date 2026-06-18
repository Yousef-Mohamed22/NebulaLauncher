import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    id: root

    required property string icon
    required property string name
    property var alignment: Text.AlignLeft

    signal clicked

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
        anchors.fill: background
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.clicked()
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
                text: root.icon
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
                width: parent.width - (root.alignment == Qt.AlignHCenter ? 32 : 0)
                anchors.verticalCenter: parent.verticalCenter
                text: root.name
                font.family: Theme.font
                font.pixelSize: 16
                font.weight: 500
                elide: Qt.ElideRight
                horizontalAlignment: root.alignment
                color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
            }
        }

    }

}
