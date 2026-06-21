pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

NblDialog {

    id: root
    property color selectedColor: Theme.colors.activeColor

    NblCheckBox {
        id: darkThemeBox
        text: "Dark Theme"
        checked: !Theme.colors.lightMode
    }

    Item {

        width: parent.width
        height: 48

        RowLayout {

            id: layout
            anchors.fill: parent
            spacing: 8

            property int selectedIndex: 0
            property list<color> swatches: [ "#d658fc", "#4083ef", "#40efe4", "#4af755", "#f2ec3a", "#f2b43a", "#f23a65", "#f96bdf" ]

            Repeater {

                id: repeater

                model: [...layout.swatches, "#000000" ]

                Item {

                    required property color modelData
                    required property int index

                    property bool isCustom: index == layout.swatches.length

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        visible: parent.index == layout.selectedIndex
                        anchors.fill: parent
                        color: "transparent"
                        border.width: 2
                        radius: 8
                        border.color: swatch.color
                    }

                    Rectangle {
                        id: swatch
                        anchors.fill: parent
                        anchors.margins: 4
                        color: parent.isCustom ? root.selectedColor : parent.modelData
                        radius: 4
                    }

                    Label {
                        visible: parent.isCustom
                        text: "Custom"
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
                            layout.selectedIndex = parent.index
                            if(parent.isCustom) {

                                let globalPos = root.mapFromItem(this, x, y)
                                let targetPos = Qt.point(
                                    globalPos.x + (width / 2) - (colorPickerPopup.popupWidth / 2),
                                    globalPos.y + height + 8
                                )

                                colorPickerPopup.open(targetPos)

                            } else {
                                root.selectedColor = parent.modelData
                                colorPicker.color = parent.modelData
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        let i = 0;
        for( ; i < layout.swatches.length ; i++) {
            let swatchColor = layout.swatches[i]
            if(swatchColor.toString() == root.selectedColor.toString()) {
                layout.selectedIndex = i
                break
            }
        }
        if(i == layout.swatches.length) {
            layout.selectedIndex = layout.swatches.length
        }
    }

    onApply: {
        Theme.colors.lightMode = !darkThemeBox.checked
        Theme.colors.activeColor = root.selectedColor
    }

    above: [ 
        NblPopupWindow {
            
            id: colorPickerPopup

            width: root.width
            height: root.height

            popupWidth: 250
            popupHeight: 250      

            Rectangle {
                id: background
                anchors.fill: parent
                color: Theme.colors.layer0
                border.width: 2
                border.color: Theme.colors.border
                radius: 8
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: false
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8

                NblColorPicker {

                    id: colorPicker

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: root.selectedColor

                    onColorChanged: {
                        root.selectedColor = color
                    }
                }
            }
        } 
    ]
}
