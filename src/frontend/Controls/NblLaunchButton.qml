import QtQuick
import QtQuick.Controls

Item {

    id: root

    required property int gameId
    required property int installationId
    required property MainWindow mainWindow
    property NblProcessHandle handle

    onInstallationIdChanged: {
        handle = ApplicationContext.getRunningInstallation(installationId)
    }

    implicitHeight: 72
    implicitWidth: 212

    enum State {
        NOT_INSTALLED,
        NOT_RUNNING,
        STARTING,
        RUNNING,
        STOPPING
    }

    property int processState: getState()

    function getState() {

        if(installationId == 0) return NblLaunchButton.State.NOT_INSTALLED

        if(!handle) return NblLaunchButton.State.NOT_RUNNING
        if(handle.isRunning) {
            if(handle.isStopping) {
                return NblLaunchButton.State.STOPPING 
            } else {
                return NblLaunchButton.State.RUNNING
            }
        } else {
            return NblLaunchButton.State.STARTING
        }
    }

    Rectangle {

        id: background

        height: parent.height
        width: parent.width

        property bool textVisible: width > 150

        color: (root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) ? Theme.colors.layer0 : Theme.colors.accent
        radius: 8

        Row {

            visible: root.processState === NblLaunchButton.State.NOT_INSTALLED

            spacing: 8
            anchors.centerIn: parent

            Label {
                text: ""
                font.family: Theme.nerdFont
                font.pixelSize: 28
                color: Theme.colors.textOnAccent
            }
            Label {
                visible: background.textVisible
                text: "INSTALL"
                font.family: Theme.font
                font.pixelSize: 28
                font.bold: true
                color: Theme.colors.textOnAccent
            }
        }

        Row {

            visible: root.processState === NblLaunchButton.State.NOT_RUNNING

            spacing: 8
            anchors.centerIn: parent

            Label {
                text: ""
                font.family: Theme.nerdFont
                font.pixelSize: 28
                color: Theme.colors.textOnAccent
            }
            Label {
                visible: background.textVisible
                text: "LAUNCH"
                font.family: Theme.font
                font.pixelSize: 28
                font.bold: true
                color: Theme.colors.textOnAccent
            }
        }

        Row {

            visible: root.processState === NblLaunchButton.State.RUNNING

            spacing: 8
            anchors.centerIn: parent

            Label {
                text: ""
                font.family: Theme.nerdFont
                font.pixelSize: 28
                color: Theme.colors.textOnAccent
            }
            Label {
                visible: background.textVisible
                text: "STOP"
                font.family: Theme.font
                font.pixelSize: 28
                font.bold: true
                color: Theme.colors.textOnAccent
            }
        }

        Row {

            visible: root.processState === NblLaunchButton.State.STARTING

            spacing: 8
            anchors.centerIn: parent

            Label {
                visible: !background.textVisible
                text: ""
                font.family: Theme.nerdFont
                font.pixelSize: 28
                color: Theme.colors.textDim
            }

            Label {
                visible: background.textVisible
                text: "Starting..."
                font.family: Theme.font
                font.pixelSize: 20
                font.weight: 600
                color: Theme.colors.textDim
            }
        }

        Row {

            visible: root.processState === NblLaunchButton.State.STOPPING

            spacing: 8
            anchors.centerIn: parent

            Label {
                visible: !background.textVisible
                text: ""
                font.family: Theme.nerdFont
                font.pixelSize: 28
                color: Theme.colors.textDim
            }

            Label {
                visible: background.textVisible
                text: "Stopping..."
                font.family: Theme.font
                font.pixelSize: 20
                font.weight: 600
                color: Theme.colors.textDim
            }
        }
    }

    Timer {
        interval: 1000
        repeat: true
        running: true

        onTriggered: {

            if(root.handle) {
                root.handle.check()
            }
        }
    }

    Connections {
        target: root.handle
        function onStopped() {
            root.handle = null
        }
    }

    MouseArea {
        anchors.fill: background
        cursorShape: (root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) ? Qt.ArrowCursor : Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: {
            if(root.processState === NblLaunchButton.State.NOT_INSTALLED) {
                root.mainWindow.beginAddInstallation(root.gameId)
            }
            else if(root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) return
            else if(root.processState === NblLaunchButton.State.RUNNING) {
                root.handle.stop()
            } else {
                root.handle = ApplicationContext.launchInstallation(root.installationId)
            }
        }
    }
}

