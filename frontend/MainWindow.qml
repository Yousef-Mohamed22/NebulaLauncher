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

            StackView {
                id: pageLoader
                Layout.fillWidth: true
                Layout.fillHeight: true

                popEnter: null
                popExit: null
                pushEnter: null
                pushExit: null
                replaceEnter: null
                replaceExit: null
            }
        }
    }

    Component.onCompleted: {
        ApplicationContext.reload()

        if(ApplicationContext.allTabs.length > 0) {
            openTab(ApplicationContext.allTabs[0])
        }
    }

    function openTab(ref) {

        while(pageLoader.depth > 1) {
            pageLoader.pop()
        }

        if(pageLoader.depth == 1 && pageLoader.currentItem.tab && pageLoader.currentItem.tab.tabId == ref.tabId) {
            return
        }

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

        pageLoader.replace("NblTabPage.qml", {
            tab: ref,
            mainWindow: root
        })
    }

    function openInstallationDetails(installationId) {

        while(pageLoader.depth > 1) {
            pageLoader.pop()
        }

        let details = ApplicationContext.getInstallationDetails(installationId)

        pageLoader.push("NblInstallationPage.qml", {
            details: details
        })
    }

}
