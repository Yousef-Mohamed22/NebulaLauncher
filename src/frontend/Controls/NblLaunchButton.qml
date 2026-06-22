import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {

    id: root

    required property int installationId
    property NblProcessHandle handle

    height: 72
    width: 212

    enum State {
        NOT_RUNNING,
        STARTING,
        RUNNING,
        STOPPING
    }

    property int processState: getState()

    function getState() {

        if(!handle) return NblLaunchButton.State.NOT_RUNNING
        if(!handle.hasStarted) {
            return NblLaunchButton.State.STARTING
        }
        if(handle.isRunning) {
            if(handle.isStopping) {
                return NblLaunchButton.State.STOPPING 
            } else {
                return NblLaunchButton.State.RUNNING
            }
        }

        return NblLaunchButton.State.NOT_RUNNING
    }


    RectangularShadow {

        width: parent.width - 4
        height: parent.height - 4
        anchors.centerIn: parent
        radius: 8

        spread: 4
        color: Theme.colors.shadow

    }

    Rectangle {

        id: background

        height: parent.height
        width: parent.width

        color: (root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) ? Theme.colors.layer0 : Theme.colors.accent
        radius: 8

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

    MouseArea {
        anchors.fill: background
        cursorShape: (root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) ? Qt.ArrowCursor : Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: {
            if(root.processState === NblLaunchButton.State.STOPPING || root.processState === NblLaunchButton.State.STARTING) return
            if(root.processState === NblLaunchButton.State.RUNNING) {
                root.handle.stop()
            } else {
                root.handle = ApplicationContext.launchInstallation(root.installationId)
            }
        }
    }
}

