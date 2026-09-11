pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root
    required property NblGameDetails details
    
    property int selectedInstallation: installationPicker.selectedInstallationId
    property NblInstallationDetails installationDetails

    onInstallationDetailsChanged: {
        installationPicker.selectedInstallation = installationDetails
    }

    property int logoPosition: installationDetails == null ? details.logoPosition : installationDetails.resolvedLogoPosition
    property string logo: installationDetails == null ? details.logoUrl() : installationDetails.resolvedLogoUrl
    property string banner: installationDetails == null ? details.bannerUrl() : installationDetails.resolvedBannerUrl

    property bool isFavorite: false

    signal closeRequested
    signal tagClicked(int id, string name, color color)

    function refresh() {
        isFavorite = false
        details = ApplicationContext.getGameDetails(details.gameId)
        installationPicker.refresh()
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

            NblCrossfadeImage {
                id: banner
                source: root.banner
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
                maskThresholdMin: 0.5
                maskThresholdMax: 1.0
            }

            // Logo image
            Item {

                height: parent.height
                width: 256
                visible: root.logoPosition < 3
                x: _calculateX()

                function _calculateX() {
                    if(root.logoPosition == 0) { // Left
                        return 0
                    }
                    if(root.logoPosition == 1) { // Center
                        return (parent.width / 2) - (width / 2)
                    }
                    if(root.logoPosition == 2) { // Right
                        return parent.width - width
                    }
                    return 0
                }

                Behavior on x {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }

                NblCrossfadeImage {
                    id: logo
                    source: root.logoPosition >= 3 ? "" : root.logo
                    anchors.centerIn: parent
                    width: parent.width - 16
                    height: parent.height - 16
                    fillMode: Image.PreserveAspectFit
                }
            }
        }

        RowLayout {

            id: launchBar

            Layout.fillWidth: true
            Layout.preferredHeight: 72
            Layout.topMargin: -24

            spacing: 8

            Rectangle {
                Layout.preferredWidth: 8
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignBottom
                radius: 8
                color: Theme.colors.layer1
            }

            // Launch Button
            Item {
                Layout.maximumWidth: 250
                Layout.minimumWidth: 48
                Layout.fillWidth: true
                Layout.preferredHeight: 72

                Rectangle {
                    width: parent.width + 16
                    height: parent.height + 16
                    x: -8
                    y: -8
                    radius: 16
                    color: Theme.colors.layer0
                    enabled: false
                }

                NblLaunchButton {
                    id: launchBtn
                    anchors.fill: parent
                    installationId: root.selectedInstallation
                    gameId: root.details.gameId
                    onInstallationAdded: {
                        root.refresh()
                    }
                }                
            }

            // Installation picker
            NblInstallationPicker {
                id: installationPicker
                gameId: root.details.gameId
                Layout.maximumWidth: 250
                Layout.minimumWidth: 48
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignBottom

                onSelectedInstallationChanged: {
                    root.installationDetails = selectedInstallation
                }
            }
            Rectangle {
                visible: root.width > 808
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignBottom
                radius: 8
                color: Theme.colors.layer1
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


                        let favoriteTagId = ApplicationContext.findTagId("favorite")

                        let rc = 0
                        if(root.isFavorite) {
                            rc = ApplicationContext.untagGame(root.details.gameId, favoriteTagId)
                        } else {
                            rc = ApplicationContext.tagGame(root.details.gameId, favoriteTagId)
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
                        ApplicationContext.openInstallationFolder(root.selectedInstallation)
                    }
                }

                NblIconButton {
                    Layout.fillWidth: true
                    icon: "󰆍"
                    onClicked: {
                        let dialog = DialogManager.create("Pages/StdoutDialog.qml", { 
                            process: launchBtn.handle
                        }) as NblDialog
                        dialog.open()
                    }
                }

                NblIconButton {
                    Layout.fillWidth: true
                    icon: ""
                    onClicked: {
                        let dialog = DialogManager.create("Pages/EditInstallationDialog.qml", { 
                            gameId: root.details.gameId,
                            installationId: root.selectedInstallation
                        }) as NblDialog
                        dialog.apply.connect(function() {
                            root.refresh()
                        })
                        dialog.open()
                    }
                }

                NblIconButton {
                    Layout.fillWidth: true
                    icon: ""
                    onClicked: {
                        DialogManager.openDeleteConfirmation( "Are you sure you want to delete this game and all of its installations?", function() {
                            ApplicationContext.deleteGame(root.details.gameId)
                            root.closeRequested()
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
                    text: root.details.title
                }

                Label {
                    width: parent.width

                    font.family: Theme.font
                    font.pixelSize: 16
                    font.weight: 400
                    wrapMode: Text.WordWrap

                    color: Theme.colors.textDim
                    text: root.details.description
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
                    text: "Release Date: " + Theme.formatDate(root.details.releaseDate)
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Added: " + Theme.formatDate(root.details.added)
                }

                Label {
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Last played: " + (root.details.lastPlayed == 0 ? "Never" : Theme.formatDate(root.details.lastPlayed))
                }

                Label {
                    visible: root.details.rating > 0
                    font.family: Theme.font
                    font.pixelSize: 14
                    font.weight: 600
                    color: Theme.colors.textDim
                    text: "Rating: " + root.details.rating.toFixed(1) + "%"
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
                        model: NblGameTagModel {
                            id: tagModel
                            gameId: root.details.gameId
                            pageSize: 8
                        }

                        NblTag {
                            required property var model;
                            color: model.color
                            text: model.name
                            width: (parent.width / 2) - 4

                            Component.onCompleted: {
                                if(model.name == "favorite") root.isFavorite = true
                            }

                            onClicked: {
                                root.tagClicked(model.id, model.name, model.color)
                            }
                        }
                    }
                }
            }
        }
    }
}
