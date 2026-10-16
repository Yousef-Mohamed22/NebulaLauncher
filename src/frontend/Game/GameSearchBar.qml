pragma ComponentBehavior: Bound

import QtQuick

import Nebula

Item {
    id: root

    function forceActiveFocus() {
        textBox.forceActiveFocus()
    }

    NblTextBox {

        id: textBox
        anchors.fill: parent
        placeholderText: "Global search (/)"
        rightPadding: 80

        Keys.onReturnPressed: {
            root.searchNow()
        }

        Keys.onEscapePressed: {
            focus = false
        }
    }

    signal searchRequested(string query)

    function searchNow() {
        searchRequested(textBox.text)
    }

    Rectangle {
        anchors.right: parent.right
        anchors.margins: 8

        property bool highlighted: textBox.text.length > 0

        width: 64
        y: 8
        height: parent.height - 16

        radius: 4
        color: highlighted ? Theme.colors.accent : Theme.colors.border

        Text {
            anchors.fill: parent
            text: ""
            font.family: Theme.nerdFont
            font.pixelSize: 20
            color: parent.highlighted ? Theme.colors.textOnAccent : Theme.colors.textDim
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: parent.highlighted ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: {
                if(parent.highlighted) root.searchNow()
            }
        }
    }
}
