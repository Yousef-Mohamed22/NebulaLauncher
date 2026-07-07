pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Effects

import Nebula.Controls

Item {

    id: root

    implicitWidth: 70
    implicitHeight: 70

    property string icon: "󰊗"
    property bool isImage: false
    property bool colorize: true

    property string dbIcon: ""
    onDbIconChanged: {
        icon = iconDisplay.getDbIconImage(dbIcon);
    }

    property list<string> presetIcons: [
        "", "", "", "", "", "", "", "", "", "", "", "󰊗", "", "", "", "", "", "󰀄",
        "", "", "", "", "", "", "", "", "", "", "", "", "󰇺", "", "󰓥", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
        "", "", "", "", "", "", "", "", 
        "", "", "", "", "", "", "", 
        "", "", "", "", "", "", "",
        "", "", "󰟡", "󱎓", "󰖫", "󰜭", "", "󰮡", "", "󰯚", "", "", "", "", "", "", "", "", "", "", ""
    ]

    Rectangle {
        anchors.fill: parent
        radius: 8
        border.width: 2
        border.color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.border
        color: "transparent"
    }

    Rectangle {
        id: textBg
        width: iconText.width + 12
        height: iconText.height + 6
        color: Theme.colors.layer0
        x: 12
        y: -12
    }

    Label {
        id: iconText
        text: "Icon"
        anchors.centerIn: textBg
        font.family: Theme.font
        font.pixelSize: 16
        font.weight: 500
        color: mouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
    }

    NblInlineIcon {
        id: iconDisplay
        anchors.fill: parent
        anchors.margins: 12
        icon: root.icon
        colorize: root.colorize
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            popup.visible = true
        }
    }

    Popup {
        id: popup
        visible: false

        width: 300
        height: 300

        x: parent.height + 8

        background: Rectangle {
            radius: 8
            border.width: 2
            border.color: Theme.colors.border
            color: Theme.colors.layer1
        }

        contentItem: ColumnLayout {

            spacing: 8

            Item {

                Layout.fillWidth: true
                Layout.preferredHeight: 64

                Rectangle {
                    width: parent.height
                    height: parent.height
                    anchors.centerIn: parent
                    color: Theme.colors.layer0
                    border.width: 2
                    border.color: Theme.colors.border
                    radius: 8
                }

                TextInput {

                    id: iconInput
                    visible: !root.isImage

                    anchors.centerIn: parent
                    text: root.icon
                    font.pixelSize: 48
                    font.family: Theme.nerdFont

                    color: Theme.colors.text

                    onTextEdited: {
                        if([...text].length > 1) {
                            text = [...text][0]
                        }
                    }

                    onTextChanged: {
                        root.icon = text
                    }

                    onVisibleChanged: {
                        if(!visible) {
                            deselect()
                        }
                    }
                }

                Item {
                    visible: root.isImage
                    width: 48
                    height: 48
                    anchors.centerIn: parent
                    Image {
                        id: iconImage
                        anchors.fill: parent
                        source: root.isImage ? root.icon : ""
                        fillMode: Image.PreserveAspectFit
                        layer.enabled: true
                        visible: !root.colorize
                        smooth: true
                        mipmap: true
                    }
                    MultiEffect {
                        visible: root.colorize
                        anchors.fill: iconImage
                        source: iconImage
                        colorization: 1.0
                        colorizationColor: Theme.colors.text
                    }
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onTapped: {
                        fileDialog.open()
                    }
                }
            }

            NblCheckBox {
                visible: root.isImage
                text: "Colorize Image"
                checked: root.colorize
                onToggled: {
                    root.colorize = checked
                }
            }

            ScrollView {

                Layout.fillWidth: true
                Layout.fillHeight: true

                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                contentHeight: pickerItems.height

                Flickable {

                    anchors.fill: parent
                    clip: true
                    
                    Flow {
                        id: pickerItems
                        width: parent.width - 8
                        x: 4
                        spacing: 8
                        Repeater {
                            model: root.presetIcons
                            delegate: NblInlineIcon {
                                width: 32
                                height: 32
                                required property var modelData
                                icon: modelData

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        root.isImage = false
                                        root.icon = parent.modelData
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Icon Image"
        fileMode: FileDialog.OpenFile
        nameFilters: [ "Image files (*.png *.svg)", "All files (*)" ]

        onAccepted: {
            let icon = fileDialog.selectedFile.toString()
            if(icon.endsWith(".png") || icon.endsWith(".svg")) {
                root.icon = icon
                root.isImage = true
            } else {
                root.icon = [...icon][0]
                root.isImage = false
            }
        }

        onRejected: {

        }
    }
}
