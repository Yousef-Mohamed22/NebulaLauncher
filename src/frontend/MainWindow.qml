import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Pages

ApplicationWindow {
    visible: true
    title: "Nebula"
    color: "black"

    id: root

    font.family: Theme.font
    font.pixelSize: 16
    font.weight: 500

    property alias currentPage: pageLoader.currentItem

    Rectangle {
        anchors.fill: parent
        color: Theme.colors.layer0
    }

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

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
                Layout.fillHeight: true

                onTabClicked: (tabId) => {
                    root.openTab(tabId)
                }
                onTabEdited: (tabId) => {
                    let tabPage = pageLoader.currentItem as NblTabPage
                    if(tabPage && tabPage.tabId == tabId) {
                        tabPage.refresh()
                    } 
                }
                onGameAdded: function(id) {
                    root.openGameDetails(id)
                }
            }
        }
    }

    Item {
        id: dialogParent
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
        target: pageLoader.currentItem
        ignoreUnknownSignals: true
        function onCloseRequested() {
            while (pageLoader.depth > 1) {
                pageLoader.pop()
            }

            let tabPage = pageLoader.currentItem as NblTabPage
            if(tabPage) {
                tabPage.refresh()
            }
        }
        function onInstallationClicked(installationId) {
            root.openInstallationDetails(installationId)
        }
        function onGameClicked(gameId) {
            root.openGameDetails(gameId)
        }
        function onTagClicked(tagId, tagName, tagColor) {
            root.openPage("Pages/WithTagPage.qml", {
                tag: tagId,
                name: tagName,
                color: tagColor,
            })
        }
    }

    Component.onCompleted: {

        DialogManager.dialogParent = dialogParent

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

    function openTab(id) {

        let initDepth = pageLoader.depth
        while(pageLoader.depth > 1) {
            pageLoader.pop()
        }

        if(initDepth > 1) {
            let page = pageLoader.currentItem as NblTabPage
            if(page && page.tabId == id) {
                page.refresh()
                return
            }
        }

        pageLoader.replace("Pages/NblTabPage.qml", {
            tabId: id
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
        })
    }
 
    function openGameDetails(gameId) {

        while (pageLoader.depth > 1) {
            pageLoader.pop()
        }
        let details = ApplicationContext.getGameDetails(gameId)

        pageLoader.push("Pages/NblGamePage.qml", {
            details: details,
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
}
