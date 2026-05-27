import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    visible: true
    title: "Nebula"
    color: "black"

    id: root

    Rectangle {
        anchors.fill: parent
        color: Theme.colors.layer0
    }

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8


        /* Header */
        Header {
            sidebar: sidebar
            Layout.fillWidth: true
            Layout.preferredHeight: 64
        }

        RowLayout {
            id: rowLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            Sidebar {
                id: sidebar
                mainWindow: root
                Layout.fillHeight: true
            }

            NblTabPage {
                id: tabPage
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Component.onCompleted: {
        ApplicationContext.reload()
    }

    function openTab(ref) {
        let rc = ref.loadFilters()
        if(rc != 0) {
            console.error("Failed to load filters for tab " + ref.name + ": " + rc)
        }
        for(let filter of ref.filters) {
            let rc = filter.loadInstallations()
            if(rc != 0) {
                console.error("Failed to load installations for filter " + filter.name + ": " + rc)
            }
        }
        tabPage.tab = ref
    }

}
