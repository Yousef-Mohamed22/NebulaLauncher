import QtQuick

import QtQuick.Layouts

NblSidebarButton {

    id: root

    required property int tabId

    signal editRequested
    signal deleteRequested

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: {
            contextMenu.open()
        }
    }

    NblContextMenu {
        id: contextMenu
        height: 96
        y: 0

        contentItem: ColumnLayout {
            spacing: 0
            NblButton {
                Layout.fillWidth: true
                Layout.fillHeight: true

                icon: ""
                text: "Edit"

                onClicked: {
                    contextMenu.close()
                    root.editRequested()
                }
                radius: 2
            }
            NblButton {
                Layout.fillWidth: true
                Layout.fillHeight: true

                icon: ""
                text: "Delete"

                onClicked: {
                    contextMenu.close()
                    root.deleteRequested()
                }
                radius: 2
            }
        }
    }

}
