import QtQuick
import QtQuick.Controls

Item {

    id: root

    required property int filterId
    required property string name
    required property string icon
    required property bool multiLine
    required property bool useIcons
    required property bool showTitles

    property bool showRowTitle: true

    required property MainWindow mainWindow

    implicitHeight: content.height

    Column {

        id: content

        width: parent.width

        Row {

            visible: root.showRowTitle

            padding: 8
            spacing: 8

            Label {
                text: root.icon
                font.family: Theme.nerdFont
                font.pixelSize: 20
                color: Theme.colors.text
            }
            Label {
                text: root.name
                font.family: Theme.font
                font.pixelSize: 20
                font.bold: true
                color: Theme.colors.text
            }
        }

        NblInstallationList {

            id: list

            filterId: root.filterId
            multiLine: root.multiLine
            mainWindow: root.mainWindow
            useIcons: root.useIcons
            showTitles: root.showTitles
        }
        
    }


}
