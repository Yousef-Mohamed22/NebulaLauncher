import QtQuick
import QtQuick.Controls

import Nebula.Controls

NblDialog {

    id: root

    property NblProcessHandle process: null

    targetWidth: 1200
    targetHeight: 800

    showApplyButton: false
    showOkButton: false

    cancelText: "Back"

    navigation: [
        NblCheckBox {
            id: autoScrollBox
            anchors.verticalCenter: parent.verticalCenter
            text: "Auto Scroll"
            checked: true
            onToggled: {
                flickable.scrollToBottom()
            }
        }
    ]

    onProcessChanged: {
        if(process != null) {
            outputContent.text = process.logs
        }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: "black"
    }

    Label {
        visible: !scrollView.visible
        anchors.centerIn: parent

        text: "The installation is not running."
        color: "white"

        font.pixelSize: 20
        font.family: Theme.font
    }

    ScrollView {

        visible: root.process != null || outputContent.text.length > 0

        id: scrollView

        anchors.fill: parent
        width: parent.width
        height: parent.height - 16
        y: 8

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        contentHeight: outputContent.height

        property bool isScrolling: ScrollBar.vertical.active

        Flickable {

            id: flickable

            anchors.fill: parent
            clip: true

            property bool autoScroll: autoScrollBox.checked
            property real lastY: contentY

            Connections {
                target: root.process
                function onLogsUpdated() {
                    outputContent.text = root.process.logs
                    flickable.scrollToBottom()
                }
            }

            onHeightChanged: {
                scrollToBottom()
            }

            onContentYChanged: {
                if(scrollView.isScrolling && flickable.contentY < lastY) {
                    autoScrollBox.checked = false
                }
                lastY = contentY
            }

            function scrollToBottom() {
                if(!autoScroll) return
                if(flickable.contentHeight > flickable.height) {
                    flickable.contentY = flickable.contentHeight - flickable.height
                }
            }

            TextEdit {

                id: outputContent

                width: parent.width - 16
                x: 8

                color: "white"
                font.pixelSize: 16
                font.family: Theme.monospaceFont

                wrapMode: TextEdit.WordWrap
                readOnly: true
                selectByMouse: true
                selectionColor: Theme.colors.accent
                selectedTextColor: Theme.colors.textOnAccent
            }
        }
    }
}
