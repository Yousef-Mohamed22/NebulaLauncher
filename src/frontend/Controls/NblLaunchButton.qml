import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {

    id: root

    required property int gameId
    required property int installationId
    property NblProcessHandle handle

    property string message: "Starting..."
    property double progress: 0

    onInstallationIdChanged: {
        handle = ApplicationContext.getRunningInstallation(installationId)
    }

    signal installationAdded

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

        color: (root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) 
            ? Theme.colors.layer1
            : Theme.colors.accent
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
                text: root.message
                font.family: Theme.font
                font.pixelSize: 16
                font.weight: 500
                color: Theme.colors.textDim
            }
        }

        Item {
            visible: root.processState === NblLaunchButton.State.STARTING
            anchors.fill: background

            Item {
                id: maskShape
                anchors.fill: parent
                layer.enabled: true
                visible: false
                Rectangle {
                    anchors.fill: parent
                    radius: background.radius
                }
            }

            Rectangle {
                id: fillShape
                anchors.fill: parent

                color: "transparent"
                layer.enabled: true
                visible: false

                Rectangle {
                    width: parent.width * Math.min(1, Math.max(0, root.progress))
                    height: 8
                    anchors.bottom: parent.bottom
                    color: Theme.colors.textDim

                    Behavior on width {
                        NumberAnimation { duration: 250 }
                    }
                }
            }

            MultiEffect {
                source: fillShape
                anchors.fill: fillShape
                maskEnabled: true
                maskSource: maskShape
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
                font.pixelSize: 16
                font.weight: 500
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
            root.message = "Starting..."
            root.progress = 0
            root.handle = null
        }
        function onStatusUpdated(message) {
            root.message = message
            root.handle.check()
        }
        function onProgressUpdated(progress) {
            root.progress = progress
            root.handle.check()
        }
    }

    MouseArea {
        anchors.fill: background
        cursorShape: (root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) ? Qt.ArrowCursor : Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: {
            if(root.processState === NblLaunchButton.State.NOT_INSTALLED) {
                let dialog = DialogManager.create("Pages/AddInstallationDialog.qml", { 
                    gameId: root.gameId
                }) as NblDialog
                dialog.apply.connect(function() {
                    root.installationAdded()
                })
                dialog.open()
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

