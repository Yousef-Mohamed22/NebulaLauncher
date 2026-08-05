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

    NblNotificationColumn {

        id: notificationColumn
        width: 316
        x: root.width - width - 8
        y: root.height - height - 8
    }
    
    Connections {
        target: ApplicationContext
        function onNotification(details) {
            notificationColumn.push(details)
        }
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
            if(settings.theme.active_color != null) {
                Theme.colors.activeColor = settings.theme.active_color
            }
            if(settings.theme.light_theme != null) {
                Theme.colors.lightMode = settings.theme.light_theme
            }
            if(settings.theme.tint_ui != null) {
                Theme.colors.tintUi = settings.theme.tint_ui
            }
        }
    }
    
    function beginAddGame() {
        openDialog("Pages/AddGameDialog.qml", {}, function() {
            let tabPage = pageLoader.currentItem as NblTabPage
            if(tabPage) {
                tabPage.refresh()
            }
            let gamePage = pageLoader.currentItem as NblGamePage
            if(gamePage) {
                gamePage.refresh()
            }
        })
    }
    function beginAddInstallation(gameId) {
        openDialog("Pages/AddGameDialog.qml", {
            gameId: gameId
        }, function() {
            let tabPage = pageLoader.currentItem as NblTabPage
            if(tabPage) {
                tabPage.refresh()
            }
            let gamePage = pageLoader.currentItem as NblGamePage
            if(gamePage) {
                gamePage.refresh()
            }
        })
    }

    function beginAddTab() {
        openDialog("Pages/AddTabDialog.qml", {}, function() {
            sidebar.reloadTabs()
        })
    }

    function beginEditTab(id) {
        openDialog("Pages/AddTabDialog.qml", {
            tabId: id
        }, function() {
            sidebar.reloadTabs()
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
                    active_color: Theme.colors.activeColor, 
                    light_theme: Theme.colors.lightMode,
                    tint_ui: Theme.colors.tintUi
                }
            })
        })
    }

    function beginEditInstallation(id) {

        openDialog("Pages/EditInstallationDialog.qml", {
            "installationId": id,
            "startOnInstallation": true
        }, function() {
            let gamePage = pageLoader.currentItem as NblGamePage
            if(gamePage) {
                gamePage.refresh()
            }
        })
    }

    function beginEditGame(id) {

        openDialog("Pages/EditInstallationDialog.qml", {
            "installationId": id
        }, function() {
            let gamePage = pageLoader.currentItem as NblGamePage
            if(gamePage) {
                gamePage.refresh()
            }
        })
    }

    function openTab(id) {

        let initDepth = pageLoader.depth
        while(pageLoader.depth > 1) {
            pageLoader.pop()
        }

        if(pageLoader.depth == 1 && pageLoader.currentItem.tabId == id) {
            if(initDepth > 1) {
                pageLoader.currentItem.refresh()
            }

            return
        }

        pageLoader.replace("Pages/NblTabPage.qml", {
            tabId: id,
            mainWindow: root
        })
    }

    function openPage(name, opts) {
        while(pageLoader.depth > 1) {
            pageLoader.pop()
        }
        pageLoader.replace(name, opts)
    }

    function openInstallationDetails(installationId) {

        while (pageLoader.depth > 1) {
            pageLoader.pop()
        }
        let iDetails = ApplicationContext.getInstallationDetails(installationId)
        let gDetails = ApplicationContext.getGameDetails(iDetails.gameId)

        pageLoader.push("Pages/NblGamePage.qml", {
            details: gDetails,
            installationDetails: iDetails,
            mainWindow: root,
        })
    }
 
    function openGameDetails(gameId) {

        while (pageLoader.depth > 1) {
            pageLoader.pop()
        }
        let details = ApplicationContext.getGameDetails(gameId)

        pageLoader.push("Pages/NblGamePage.qml", {
            details: details,
            mainWindow: root,
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
