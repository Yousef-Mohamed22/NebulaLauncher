pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts


Item {

    id: root

    required property MainWindow mainWindow
    property bool collapsed: true

    implicitWidth: collapsed ? 64 : 240

    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.colors.layer1
        border.width: 2
        border.color: Theme.colors.border
        radius: 8
    }

    ColumnLayout {
        anchors.fill: background
        anchors.margins: 8
        spacing: 8

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 8

            Repeater {
                model: ApplicationContext.allTabs
                
                NblTabButton {
                    required property NblTabRef modelData

                    Layout.fillWidth: true

                    mainWindow: root.mainWindow
                    collapsed: root.collapsed
                    ref: modelData
                }
            }

        }

    }

}
