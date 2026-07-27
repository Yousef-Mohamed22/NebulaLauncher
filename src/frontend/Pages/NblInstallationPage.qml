pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root
    required property MainWindow mainWindow
    required property NblInstallationDetails details

    property bool isFavorite: false

    function refresh() {
        details = ApplicationContext.getInstallationDetails(details.installationId)
    }

    GridLayout {

        width: parent.width
        height: parent.height

        rows: 3
        columns: 2

        columnSpacing: 8
        rowSpacing: 8

        uniformCellWidths: false
        uniformCellHeights: false

        // Banner
        Item {

            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 240

            Image {
                id: banner
                source: root.details.bannerUrl()
                fillMode: Image.PreserveAspectCrop
                width: parent.width
                height: parent.height
                visible: false
                layer.enabled: true
            }

            Item {
                id: maskShape
                width: banner.width
                height: banner.height
                visible: false
                layer.enabled: true
                Rectangle {
                    anchors.fill: maskShape
                    radius: 8
                }
            }

            MultiEffect {
                source: banner
                anchors.fill: banner
                maskEnabled: true
                maskSource: maskShape
            }

            // Logo image
            Item {

                height: parent.height
                width: 256

                visible: root.details.logoPosition < 3

                anchors.left: root.details.logoPosition == 0 ? parent.left : undefined
                anchors.horizontalCenter: root.details.logoPosition == 1 ? parent.horizontalCenter : undefined
                anchors.right: root.details.logoPosition == 2 ? parent.right : undefined

                Image {
                    id: logo
                    source: root.details.logoUrl()
                    anchors.centerIn: parent
                    width: parent.width - 16
                    height: parent.height - 16
                    fillMode: Image.PreserveAspectFit
                }
            }
        }

        // Launch Button
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 48

            NblLaunchButton {
                id: launchBtn
                x: 12
                y: -24
                installationId: root.details.installationId
            }
        }

        // Actions
        Item {

            Layout.preferredWidth: 272
            Layout.preferredHeight: 48

            RowLayout {
                anchors.fill: parent

                spacing: 8

                NblIconButton {
                    Layout.fillWidth: true
                    icon: ""
                    colorUnhovered: root.isFavorite ? Theme.colors.accent : Theme.colors.textDim
                    onClicked: {

                        let rc = 0
                        if(root.isFavorite) {
                            rc = ApplicationContext.untagInstallation(root.details.installationId, "favorite")
                        } else {
                            rc = ApplicationContext.tagInstallation(root.details.installationId, "favorite")
                        }
                        if(rc != 0) {
                            ApplicationContext.pushNotification({
                                type: "error",
                                title: "Unable to mark as favorite",
                                message: "Does the \"favorite\" tag exist?"
                            })
                        } else {
                            root.isFavorite = !root.isFavorite
                            tagModel.refresh()
                        }
                    }
                }

                NblIconButton {
                    Layout.fillWidth: true
                    icon: ""
                    onClicked: {
                        ApplicationContext.openInstallationFolder(root.details.installationId)
                    }
                }

                NblIconButton {
                    Layout.fillWidth: true
                    icon: "󰆍"
                    onClicked: {
                        root.mainWindow.openDialog("Pages/StdoutDialog.qml", {
                            process: launchBtn.handle
                        })
                    }
                }

                NblIconButton {
                    Layout.fillWidth: true
                    icon: ""
                    onClicked: {
                        root.mainWindow.beginEditGame(root.details)
                    }
                }

                NblIconButton {
                    Layout.fillWidth: true
                    icon: ""
                    onClicked: {
                        root.mainWindow.openDialog("Pages/ConfirmDeleteDialog.qml", {
                            "text": "Are you sure you want to delete this installation?"
                        }, function() {
                            ApplicationContext.deleteInstallation(root.details.installationId)
                            root.mainWindow.closeInstallationDetails()
                        })
                    }
                }
            }
        }

        // Title / Description
        Item {

            Layout.fillHeight: true
            Layout.fillWidth: true

            Column {

                anchors.fill: parent
                anchors.margins: 16

                spacing: 16

                Label {
                    font.family: Theme.font
                    font.pixelSize: 24
                    font.weight: 500
                    color: Theme.colors.text
                    text: root.details.gameTitle + (root.details.title.byteLength > 0 ? (" - " + root.details.title) : "")
                }

                Label {
                    width: parent.width

                    font.family: Theme.font
                    font.pixelSize: 16
                    font.weight: 400
                    wrapMode: Text.WordWrap

                    color: Theme.colors.textDim
                    text: root.details.description
                    visible: root.details.description.byteLength > 0
                }

                Label {
                    width: parent.width

                    font.family: Theme.font
                    font.pixelSize: 16
                    font.weight: 400
                    wrapMode: Text.WordWrap

                    color: Theme.colors.textDim
                    text: root.details.gameDescription
                }
            }
        }

        // Tags and other details
        Item { 
            Layout.fillHeight: true
            Layout.preferredWidth: 272

            Rectangle {
                anchors.fill: parent
                color: Theme.colors.layer1
                border.width: 2
                border.color: Theme.colors.border
                radius: 8
            }

            Column {

                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                function _formatDate(timeUint) {
                    return Qt.formatDate(new Date(timeUint), "yyyy/MM/dd")
                }

                function _formatElapsedTime(totalMs) {
                    let totalSeconds = Math.floor(totalMs / 1000);
                    let seconds = totalSeconds % 60;
                    let minutes = Math.floor(totalSeconds / 60) % 60;
                    let hours = Math.floor(totalSeconds / 3600);

                    let pad = (num) => String(num).padStart(2, '0');

                    return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}`;
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 16
                    font.weight: 600
                    color: Theme.colors.text
                    text: "Metadata"
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Release Date: " + parent._formatDate(root.details.releaseDate)
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Rating: " + root.details.rating
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Play time: " + parent._formatElapsedTime(root.details.playTime)
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Play count: " + root.details.playCount
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Last played: " + parent._formatDate(root.details.lastPlayed)
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 16
                    font.weight: 600
                    color: Theme.colors.text
                    text: "Tags"
                    topPadding: 12
                }

                Flow {

                    width: parent.width
                    spacing: 8

                    Repeater {
                        model: NblInstallationTagModel {
                            id: tagModel
                            installationId: root.details.installationId;
                            pageSize: 8
                        }

                        NblTag {
                            required property var model;
                            color: model.color
                            text: model.name

                            Component.onCompleted: {
                                if(model.name == 'favorite') root.isFavorite = true
                            }
                        }
                    }
                }
            }
        }
    }
}
