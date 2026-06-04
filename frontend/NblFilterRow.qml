import QtQuick
import QtQuick.Controls

Item {

    id: root

    required property NblFilterRef filter;
    required property MainWindow mainWindow

    implicitHeight: content.height

    Column {

        id: content

        width: parent.width

        Row {

            padding: 8
            spacing: 8

            Label {
                text: root.filter.icon
                font.family: Theme.nerdFont
                font.pixelSize: 20
                color: Theme.colors.text
            }
            Label {
                text: root.filter.name
                font.family: Theme.font
                font.pixelSize: 20
                font.bold: true
                color: Theme.colors.text
            }
        }

        NblInstallationList {
            filter: root.filter
            mainWindow: root.mainWindow
        }
        
    }


}
