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
        modalLoader.setSource("Pages/AddGameDialog.qml", { 
            "mainWindow": root,
            "showApplyButton": false
        })
        modalConnections.applyAction = function() { reloadTab() }
        modalLoader.active = true
    }

    function openSettings() {
        modalLoader.setSource("Pages/AppSettingsDialog.qml", { 
            "mainWindow": root
        })
        modalConnections.applyAction = function() { 
            ApplicationContext.saveSettings({
                theme: {
                    activeColor: Theme.colors.activeColor, 
                    lightTheme: Theme.colors.lightMode
                }
            })
        }
        modalLoader.active = true
    }

    function beginEditGame(details) {
        modalLoader.setSource("Pages/AddGameDialog.qml", {
            "mainWindow": root,
            "activeInstallation": details
        })
        modalConnections.applyAction = function() { reloadTab() }
        modalLoader.active = true
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

        pageLoader.replace("Pages/NblTabPage.qml", {
            tab: ref,
            mainWindow: root
        })

    }

    function openInstallationDetails(installationId) {

        while(pageLoader.depth > 1) {
            pageLoader.pop()
        }

        let details = ApplicationContext.getInstallationDetails(installationId)

        pageLoader.push("Pages/NblInstallationPage.qml", {
            details: details,
            mainWindow: root
        })
    }

}
