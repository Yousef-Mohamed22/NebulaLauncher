pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {

    id: root

    required property NblTabRef tab
    required property MainWindow mainWindow

    ListView {

        width: 1000
        height: 1000


        model: NblTabModel {
            id: tabModel
            pageSize: 10
            tabId: root.tab.tabId
        }

        delegate: NblFilterRow {
            required property var model
            width: parent.width
            filterId: model.id
            name: model.name
            icon: model.icon
            multiLine: model.multiLine
            showTitles: model.showTitles
            useIcons: model.useIcons

            mainWindow: root.mainWindow
        }

        onAtYEndChanged: {
            if(atYEnd && !tabModel.isLoading) {
                tabModel.loadNextPage()
            }
        }
    }

    function refresh() {
        tabModel.refresh()
    }

}
