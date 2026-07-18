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

    Item {

        width: 316
        height: root.height - 16

        x: root.width - width - 8
        y: root.height - height - 8

        enabled: false

        ListModel {
            id: activeNotifications
        }

        ListView {
            anchors.fill: parent
            model: activeNotifications

            verticalLayoutDirection: ListView.BottomToTop

            spacing: 8

            delegate: Item {
                width: 316
                height: 128

                id: notif

                required property int index
                required property string type
                required property string title
                required property string message

                Rectangle {
                    id: notifBg
                    anchors.fill: parent
                    color: parent.type == "error" ? "#af1f43" : Theme.colors.layer1
                    radius: 8
                }

                ColumnLayout {

                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    Label {
                        Layout.fillWidth: true
                        text: notif.title
                        font.family: Theme.font
                        font.pixelSize: 16
                        font.bold: true
                        color: Theme.colors.blackOrWhite(notifBg.color)
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: notif.message
                        font.family: Theme.font
                        font.pixelSize: 16
                        color: Theme.colors.blackOrWhite(notifBg.color)
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                    }
                }

                Timer {
                    running: true
                    repeat: false
                    interval: 5000

                    onTriggered: {
                        activeNotifications.remove(parent.index)
                    }
                }
            }
        }
    }

    Connections {
        target: ApplicationContext
        function onNotification(details) {
            activeNotifications.append(details)
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
            if(settings.theme.activeColor) {
                Theme.colors.activeColor = settings.theme.activeColor
            }
            if(settings.theme.lightTheme) {
                Theme.colors.lightMode = settings.theme.lightTheme
            }
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

    function openInstallationDetails(installationId) {

        while (pageLoader.depth > 1) {
            pageLoader.pop()
        }
        let details = ApplicationContext.getInstallationDetails(installationId)

        pageLoader.push("Pages/NblInstallationPage.qml", {
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
