pragma ComponentBehavior: Bound

import QtQuick
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

    RowLayout {

        spacing: 8
        width: parent.width
        height: 48

        Repeater {

            // Built-in swatches
            model: [ "#d658fc", "#4083ef", "#40efe4", "#4af755", "#f2ec3a", "#f2b43a", "#f23a65", "#f96bdf" ] 

            Item {

                required property color modelData

                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    visible: root.selectedColor == parent.modelData
                    anchors.fill: parent
                    color: "transparent"
                    border.width: 2
                    radius: 8
                    border.color: parent.modelData
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 4
                    color: parent.modelData
                    radius: 4
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.selectedColor = parent.modelData
                    }
                }
            }
        }
    }

    // TODO: Full color picker

    onApply: {
        Theme.colors.lightMode = !darkThemeBox.checked
        Theme.colors.activeColor = root.selectedColor
    }

}
