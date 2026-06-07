pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic

ComboBox {

    id: root
    required property string label


    implicitHeight: 56
    y: 4

    background: Rectangle {
        color: Theme.colors.layer0
        height: parent.height - 8
        y: 4
        radius: 8
        border.width: 2
        border.color: root.popup.visible ? Theme.colors.accent : Theme.colors.border
    }

    contentItem: Text {
        text: root.displayText
        font.pixelSize: 16
        font.family: Theme.font
        color: Theme.colors.text
        topPadding: 16
        leftPadding: 16
        rightPadding: 16
    }
    
    popup: Popup {
        y: root.height - 1
        width: root.width
        height: Math.min(contentItem.implicitHeight + 2, root.Window.height - topMargin - bottomMargin)
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            color: Theme.colors.layer1
            border.color: Theme.colors.border
            border.width: 2
            radius: 8
        }
    }

    delegate: ItemDelegate {
        id: delegate

        required property var model
        required property int index

        width: root.width
        contentItem: Text {
            text: delegate.model[root.textRole]
            color: Theme.colors.text
            font.family: Theme.font
            font.pixelSize: 16
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: delegate.highlighted ? Theme.colors.border : "transparent"
            radius: 4
        }
        highlighted: root.highlightedIndex === index
    }

    Item {

        width: 0
        height: 0
        x: 16
        y: -6

        Rectangle { 
            width: labelText.width + 12
            height: labelText.height - 8
            anchors.centerIn: labelText
            color: Theme.colors.layer0 
        }
        Label { 
            id: labelText
            text: root.label
            font.pixelSize: 16
            font.family: Theme.font
            font.weight: 500
            color: root.popup.visible ? Theme.colors.accent : Theme.colors.textDim
        }
    }

}
