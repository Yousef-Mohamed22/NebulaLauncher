pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

Column {

    property color selectedColor: Theme.colors.activeColor
    readonly property bool darkTheme: darkThemeBox.checked

    id: root
    spacing: 16

    NblCheckBox {
        id: darkThemeBox
        text: "Dark Theme"
        checked: !Theme.colors.lightMode
    }

    Item {

        Label {
            text: "Accent Color"
            font.family: Theme.font
            font.pixelSize: 16
            color: Theme.colors.text
        }

        width: parent.width
        height: 72

        RowLayout {

            id: layout
            spacing: 8

            y: 24
            width: parent.width
            height: parent.height - 24


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

            property int selectedIndex: 0
            property list<color> swatches: [ "#d658fc", "#4083ef", "#40efe4", "#4af755", "#f2ec3a", "#f2b43a", "#f23a65", "#f96bdf" ]

            Repeater {

                id: repeater
                model: layout.swatches

                Item {

                    id: swatch

                    required property color modelData
                    required property int index

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        visible: parent.index == layout.selectedIndex
                        anchors.fill: parent
                        color: "transparent"
                        border.width: 2
                        radius: 8
                        border.color: parent.modelData
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 4
                        color:  parent.modelData
                        radius: 4
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.selectedColor = parent.modelData
                            layout.selectedIndex = parent.index
                        }
                    }
                }
            }

            NblColorSwatch {

                id: customColor
                label: "Custom"

                selectedColor: root.selectedColor

                Layout.fillWidth: true
                Layout.fillHeight: true

                onSelectedColorChanged: {
                    root.selectedColor = selectedColor
                    layout.selectedIndex = layout.swatches.length
                }

                Rectangle {
                    visible: layout.selectedIndex == layout.swatches.length
                    anchors.fill: parent
                    color: "transparent"
                    border.width: 2
                    radius: 8
                    border.color: parent.selectedColor
                }
            }
        }
    }
}

