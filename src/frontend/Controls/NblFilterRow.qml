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

    implicitHeight: content.height

    signal installationClicked(int id)

    Column {

        id: content
        width: parent.width

        Row {

            x: 8

            visible: root.showRowTitle
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
            width: content.width

            filterId: root.filterId
            multiLine: root.multiLine
            useIcons: root.useIcons
            showTitles: root.showTitles

            onInstallationClicked: id => {
                root.installationClicked(id)
            }
        }
        
    }

    function scrolled() {
        list.scrolled()
    }

}
