import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root
    required property MainWindow mainWindow
    required property NblInstallationDetails details

    function refresh() {
        details = ApplicationContext.getInstallationDetails(details.installationId)
    }

    ColumnLayout {

        width: parent.width
        height: parent.height
        spacing: 8

        Item {

            Layout.fillWidth: true
            Layout.preferredHeight: 240

            implicitHeight: 240

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


        RowLayout {

            Layout.leftMargin: 8
            Layout.fillHeight: true

            spacing: 8

            Column {

                Layout.fillHeight: true
                Layout.fillWidth: true

                spacing: 16

                RowLayout {

                    width: parent.width
                    height: 48

                    anchors.leftMargin: 8
                    spacing: 8

                    NblLaunchButton {
                        Layout.topMargin: -24
                        details: root.details
                    }

                    Item { Layout.fillWidth: true }

                    NblIconButton {
                        icon: ""
                        onClicked: {
                            console.log("Clicked favorite")
                        }
                    }

                    NblIconButton {
                        icon: ""
                        onClicked: {
                            console.log("Clicked Folder")
                        }
                    }

                    NblIconButton {
                        icon: ""
                        onClicked: {
                            root.mainWindow.beginEditGame(root.details)
                        }
                    }

                    NblIconButton {
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

            Item {

                Layout.fillHeight: true
                Layout.preferredWidth: 300

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

                    Label {
                        font.family: Theme.font
                        font.pixelSize: 16
                        font.weight: 600
                        color: Theme.colors.text
                        text: "Tags"
                    }

                    Flow {

                        width: parent.width
                        spacing: 8

                        Repeater {
                            model: NblInstallationTagModel {
                                installationId: root.details.installationId;
                                pageSize: 8
                            }
                        
                            NblTag {
                                required property var model;
                                color: model.color
                                text: model.name
                            }
                        }
                    }
                }
            }
        }
    }
}
