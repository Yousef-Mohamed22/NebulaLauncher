pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {

    id: root

    implicitHeight: view.contentHeight

    property int notificationStart: width + 16
    property int notificationHeight: 108

    function push(details) {
        if(details.icon == null) {
            if(details.type == "error") {
                details.icon = ""
            } else {
                details.icon = ""
            }
        }

        if(details.color == null) {
            if(details.type == "error") {
                details.color = "#af1f43"
            } else {
                details.color = Theme.colors.layer1
            }
        }
        if(details.duration == null) details.duration = 5000

        activeNotifications.insert(0, details)
    }

    ListModel {
        id: activeNotifications
    }

    ListView {

        id: view

        width: parent.width
        height: contentHeight

        interactive: false

        model: activeNotifications

        verticalLayoutDirection: ListView.BottomToTop

        spacing: 8

        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 150; easing.type: Easing.OutQuad; }
        }
        delegate: Item {
            id: notif

            required property int index
            required property string title
            required property string message
            required property string icon
            required property color color
            required property int duration

            property int renderX: root.notificationStart

            property bool dismissed: false

            width: root.width
            height: root.notificationHeight

            property int remainingMillis: duration

            onDismissedChanged: {
                if(dismissed) {
                    notif.renderX = root.notificationStart
                }
            }

            Timer {
                running: true
                repeat: false
                interval: 1
                onTriggered: {
                    notif.renderX = 0
                }
            }

            Timer {
                id: autoRemoveTimer

                property int prevTime: Date.now()

                running: notif.duration
                repeat: true
                interval: 100
                onTriggered: {

                    let now = Date.now()
                    let elapsed = now - prevTime
                    prevTime = now
                    if(notifArea.containsMouse || notifXArea.containsMouse) return

                    notif.remainingMillis -= elapsed
                    if(notif.remainingMillis <= 0) {
                        if(notif.index != -1) {
                            notif.dismissed = true
                        }
                    }
                }
            }

            Timer {
                running: notif.dismissed
                interval: 250
                repeat: false
                onTriggered: {
                    if(notif.index != -1) activeNotifications.remove(notif.index)
                }
            }

            MouseArea {
                id: notifArea
                anchors.fill: notif
                hoverEnabled: !notif.dismissed
                onContainsMouseChanged: {
                    if(notif.dismissed) return
                    notif.remainingMillis = notif.duration
                }
            }

            Item {

                width: parent.width
                height: parent.height
                x: notif.renderX

                Behavior on x {
                    SpringAnimation { spring: 3; damping: .25 }
                }

                Rectangle {
                    id: notifBg
                    anchors.fill: parent
                    color: notif.color
                    radius: 8
                    visible: false

                    Rectangle {
                        id: barShape
                        width: parent.width * (notif.remainingMillis / notif.duration)
                        height: 8
                        anchors.bottom: parent.bottom
                        color: Theme.colors.readableOn(notifBg.color)

                        Behavior on width {
                            NumberAnimation { duration: 100 }
                        }
                    }
                }

                Rectangle {
                    id: maskShape
                    width: notifBg.width
                    height: notifBg.height
                    radius: notifBg.radius
                    visible: false
                    layer.enabled: true
                }

                MultiEffect {
                    source: notifBg
                    anchors.fill: notifBg
                    maskEnabled: true
                    maskSource: maskShape
                }

                RowLayout {

                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    anchors.topMargin: 6
                    anchors.bottomMargin: 6
                    spacing: 8

                    NblInlineIcon {
                        icon: notif.icon
                        Layout.preferredWidth: 64
                        Layout.fillHeight: true
                    }
                    ColumnLayout {

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        spacing: 4

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 20

                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                Label {
                                    width: parent.width
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: notif.title
                                    font.family: Theme.font
                                    font.pixelSize: 16
                                    font.bold: true
                                    elide: Text.ElideRight
                                    color: Theme.colors.blackOrWhite(notif.color)
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                                Layout.preferredWidth: 16
                                property bool hovered
                                Label {
                                    text: ""
                                    font.family: Theme.nerdFont
                                    font.pixelSize: 16
                                    anchors.centerIn: parent
                                    color: notifXArea.containsMouse ? Theme.colors.text : Theme.colors.readableOn(notif.color)
                                }
                                MouseArea {
                                    id: notifXArea
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    hoverEnabled: !notif.dismissed
                                    onClicked: {
                                        notif.dismissed = true
                                        autoRemoveTimer.stop()
                                    }
                                }
                            }
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
                }
            }
        }
    }
}

