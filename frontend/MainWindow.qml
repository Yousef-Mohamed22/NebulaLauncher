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

    Loader {
        id: modalLoader
        anchors.fill: parent
        active: false
    }


    Component.onCompleted: {
        ApplicationContext.reload()

        if(ApplicationContext.allTabs.length > 0) {
            openTab(ApplicationContext.allTabs[0])
        }
    }

    function beginAddGame() {
        modalLoader.setSource("AddGameDialog.qml", { "mainWindow": root })
        modalLoader.active = true
    }

    function finishAddGame() {
        modalLoader.active = false
        reloadTab()
    }

    function reloadTab() {
        if(pageLoader.depth == 1 && pageLoader.currentItem.tabId) {
            pageLoader.currentItem.refresh()
        }
    }

    function openTab(ref) {

        let initDepth = pageLoader.depth
        while(pageLoader.depth > 1) {
            pageLoader.pop()
        }

        if(pageLoader.depth == 1 && pageLoader.currentItem.tabId == ref.tabId) {
            if(initDepth > 1) {
                pageLoader.currentItem.refresh()
            }

            return
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
