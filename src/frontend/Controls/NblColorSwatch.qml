import QtQuick
import QtQuick.Controls

Item {

    id: root

    property string label: ""
    property alias selectedColor: colorPicker.color

    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        color: root.selectedColor
        radius: 4
    }

    Label {
        text: root.label
        font.family: Theme.font
        font.pixelSize: 14
        font.weight: 600
        color: Theme.colors.readableOn(root.selectedColor)
        anchors.centerIn: parent
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            colorPickerPopup.open()
        }
    }

    Popup {
                            
        id: colorPickerPopup

        width: 250
        height: 250

        y: parent.height + 8

        modal: false
        clip: true
        padding: 8

        background: Rectangle {
            id: background
            anchors.fill: parent
            color: Theme.colors.layer0
            border.width: 2
            border.color: Theme.colors.border
            radius: 8
        }

        contentItem: NblColorPicker {
            id: colorPicker
        }
    } 
}

