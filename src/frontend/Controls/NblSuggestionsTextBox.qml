pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic

TextField {

    id: root
    property string label
    property string tooltip
    property int tooltipDelay: 1000

    color: enabled ? Theme.colors.text : Theme.colors.textDim
    placeholderTextColor: Theme.colors.border

    font.pixelSize: 16
    font.family: Theme.font

    implicitHeight: 54

    topPadding: 12
    leftPadding: 16
    rightPadding: 16

    background: Rectangle {
        color: Theme.colors.layer0
        height: parent.height - 6
        y: 6
        radius: 8
        border.width: 2
        border.color: root.cursorVisible ? Theme.colors.accent : Theme.colors.border
    }

    onFocusChanged: {
        if(focus && popup.suggestionsModel.count > 0) {
            popup.open()
        }
    }

    Item {

        visible: root.label && root.label.length > 0

        width: 0
        height: 0
        x: 16
        y: -6

        Rectangle { 
            width: labelText.width + 12
            height: labelText.height - 8
            anchors.centerIn: labelText
            color: Theme.colors.layer0 

            NblToolTip {
                delay: root.tooltipDelay
                text: root.tooltip
                visible: hover.containsMouse
            }

            MouseArea {
                id: hover
                anchors.fill: parent
                hoverEnabled: root.tooltip.length > 0
            }
        }
        Label { 
            id: labelText
            text: root.label
            font.family: Theme.font
            font.weight: 500
            color: root.enabled 
                ? root.cursorVisible 
                    ? Theme.colors.accent 
                    : Theme.colors.textDim
                : Theme.colors.border
        }
    }

    function clearSuggestions() {
        popup.suggestionsModel.clear()
    }

    function pushSuggestion(str) {
        popup.suggestionsModel.append({ text: str })
        if(focus && !popup.opened) {
            popup.open()
        }
    }

    onPressed: {
        if(popup.suggestionsModel.count > 0 && !popup.opened) {
            popup.open()
        }
    }

    Keys.onReturnPressed: event => {
        if(popup.suggestionsModel.count > 0) {
            root.text = popup.suggestionsModel.get(popup.selectedIndex).text
            popup.close()
        }
        event.accepted = true
    }

    Keys.onEscapePressed: event => {
        popup.close()
        event.accepted = true
    }

    Keys.onDownPressed: event => {
        popup.selectedIndex = Math.min(popup.selectedIndex + 1, popup.suggestionsModel.count - 1)
        event.accepted = true
    }
    Keys.onUpPressed: event => {
        popup.selectedIndex = Math.max(popup.selectedIndex - 1, 0)
        event.accepted = true
    }

    Popup {

        id: popup

        property ListModel suggestionsModel: ListModel {}
        property int selectedIndex: 0

        y: root.height

        width: root.width
        height: Math.min(144, (suggestionsModel.count * 32) + 16)

        padding: 8

        background: Rectangle {
            radius: 8
            border.width: 2
            border.color: Theme.colors.border
            color: Theme.colors.layer0
        }
        contentItem: ListView {
            model: popup.suggestionsModel
            delegate: Item {

                width: root.width - 16
                height: 32

                required property int index
                required property string text

                Rectangle {
                    anchors.fill: parent
                    color: popup.selectedIndex == parent.index ? Theme.colors.layer1 : "transparent"
                    radius: 4
                }
                Label {
                    anchors.fill: parent
                    anchors.margins: 4
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    elide: Text.ElideRight
                    font.family: Theme.font
                    font.pixelSize: 16
                    color: Theme.colors.text
                    text: parent.text
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: {
                        if(containsMouse) {
                            popup.selectedIndex = parent.index
                        }
                    }
                    onClicked: {
                        root.text = parent.text
                        popup.close()
                    }
                }
            }
        }
    }
}
