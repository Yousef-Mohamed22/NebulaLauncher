import QtQuick
import QtQuick.Controls

Item {

    id: root

    required property int filterId
    required property string name
    required property string icon
    required property bool colorizeIcon
    required property bool multiLine
    required property bool useIcons
    required property bool showTitles
    required property bool showInstallations
    required property bool showFilterInfo

    property bool showRowTitle: true

    implicitHeight: content.height

    signal itemClicked(int id)
    signal gameClicked(int id)
    signal installationClicked(int id)

    Column {

        id: content
        width: parent.width

        Row {

            x: 8
            height: 32
            spacing: 8

            visible: root.showRowTitle

            NblInlineIcon {
                width: 20
                height: 20
                anchors.verticalCenter: parent.verticalCenter
                dbIcon: root.icon
                color: Theme.colors.text
                colorize: root.colorizeIcon
            }
            Label {
                text: root.name
                font.family: Theme.font
                font.pixelSize: 20
                font.bold: true
                color: Theme.colors.text
            }
        }

        NblGameList {

            id: list
            width: content.width

            multiLine: root.multiLine
            useIcons: root.useIcons
            showTitles: root.showTitles
            showFilterInfo: root.showFilterInfo

            model: NblFilterModel {
                pageSize: 30
                filterId: root.filterId
                searchInstallations: root.showInstallations
            }
            onLoadMore: {
                model.loadNextPage()
                allLoaded = model.allLoaded
            }

            onItemClicked: id => {
                root.itemClicked(id)
                if(root.showInstallations) {
                    root.installationClicked(id)
                } else {
                    root.gameClicked(id)
                }
            }
        }
        
    }

    function scrolled() {
        list.scrolled()
    }

}
