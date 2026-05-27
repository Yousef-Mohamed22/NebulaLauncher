import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {

    property NblTabRef tab

    Flickable {

        id: flickable

        width: parent.width
        height: parent.height

        contentWidth: column.width
        contentHeight: column.height
        flickableDirection: Flickable.VerticalFlick
        clip: true

        Column {
            id: column
            spacing: 8
            width: flickable.width - 16

            Repeater {
                model: tab ? tab.filters : 0
                NblFilterRow {
                    required property NblFilterRef modelData

                    width: parent.width
                    filter: modelData
                }
            }
        }

        ScrollBar.horizontal: null
        ScrollBar.vertical: ScrollBar {
            parent: flickable
            anchors.top: flickable.top
            anchors.bottom: flickable.bottom
            anchors.right: flickable.right
            active: flickable.movingVertically
        }
    }
}
