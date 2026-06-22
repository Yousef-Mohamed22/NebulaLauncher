import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Pages

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

            layoutDirection: Qt.RightToLeft

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
            Sidebar {
                id: sidebar
                mainWindow: root
                Layout.fillHeight: true
            }
        }
    }

    Loader {
        id: modalLoader
        anchors.fill: parent
        active: false
    }

    Connections {

        id: modalConnections
        target: modalLoader.item

        property var applyAction

        function onClose() {
            modalLoader.active = false
        }

        function onApply() {
            applyAction()
        }
    }

    Component.onCompleted: {

        var settings = ApplicationContext.loadSettings()
        if(settings.theme) {
            if(settings.theme.activeColor) {
                Theme.colors.activeColor = settings.theme.activeColor
            }
            if(settings.theme.lightTheme) {
                Theme.colors.lightMode = settings.theme.lightTheme
            }
        }

        ApplicationContext.reload()

        if(ApplicationContext.allTabs.length > 0) {
            openTab(ApplicationContext.allTabs[0])
        }
    }
    
    function beginAddGame() {
        openDialog("Pages/AddGameDialog.qml", {}, function() {
            let tabPage = pageLoader.currentItem as NblTabPage
            if(tabPage) {
                tabPage.refresh()
            }
        })
    }

    function openSettings() {
        openDialog("Pages/AppSettingsDialog.qml", {}, function() { 
            ApplicationContext.saveSettings({
                theme: {
                    activeColor: Theme.colors.activeColor, 
                    lightTheme: Theme.colors.lightMode
                }
            })
        })
    }

    function beginEditGame(details) {

        openDialog("Pages/AddGameDialog.qml", {
            "activeInstallation": details
        }, function() {
            let installationPage = pageLoader.currentItem as NblInstallationPage
            if(installationPage) {
                installationPage.refresh()
            }
        })
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

        pageLoader.replace("Pages/NblTabPage.qml", {
            tab: ref,
            mainWindow: root
        })

    }

    function openInstallationDetails(installationId) {

        while (pageLoader.depth > 1) {
            pageLoader.pop()
        }
        let details = ApplicationContext.getInstallationDetails(installationId)

        pageLoader.push("Pages/NblInstallationPage.qml", {
            details: details,
            mainWindow: root,
            handle: ApplicationContext.getRunningInstallation(installationId)
        })
    }

    function closeInstallationDetails() {
        while (pageLoader.depth > 1) {
            pageLoader.pop()
        }

        let tabPage = pageLoader.currentItem as NblTabPage
        if(tabPage) {
            tabPage.refresh()
        }
    }

    function openDialog(file, options, applyAction) {

        let realOptions = Object.assign({}, options)
        realOptions.mainWindow = this

        modalLoader.setSource(file, realOptions)
        modalConnections.applyAction = applyAction

        modalLoader.active = true

    }

}
