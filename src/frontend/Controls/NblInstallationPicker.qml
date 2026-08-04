pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

Item { 

    id: root

    property int gameId
    property NblInstallationDetails selectedInstallation: null
    readonly property int selectedInstallationId: selectedInstallation == null ? 0 : selectedInstallation.installationId

    Rectangle {
        anchors.fill: parent
        color: Theme.colors.layer1
        radius: 8
    }

    function refresh() {
        let id = selectedInstallationId
        installationsModel.refresh()
        if(id != 0) {
            selectedInstallation = ApplicationContext.getInstallationDetails(id)
        }
    }

    RowLayout {

        visible: root.selectedInstallation != null

        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Item {

            Layout.fillHeight: true
            Layout.preferredWidth: parent.height

            NblImage {
                id: imageSource
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                layer.enabled: true
                visible: false
                source: root.selectedInstallation == null ? null : root.selectedInstallation.iconUrl()
            }

            Item {
                id: maskShape
                anchors.fill: imageSource
                visible: false
                layer.enabled: true
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                }
            }

            MultiEffect {
                anchors.fill: imageSource
                source: imageSource
                maskEnabled: true
                maskSource: maskShape
                maskThresholdMin: 0.5
                maskSpreadAtMin: 1.0
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            visible: parent.width > parent.height * 2

            Label {
                text: root.selectedInstallation == null ? "" : root.selectedInstallation.resolvedTitle
                y: -(parent.height / 7)
                width: parent.width
                color: Theme.colors.text
                font.family: Theme.font
                font.pixelSize: parent.height / 2
                font.weight: 500
                elide: Text.ElideRight
            }

            Label {
                text: "Last played: " + (root.selectedInstallation == null || root.selectedInstallation.lastPlayed == 0 ? "Never" : Theme.formatDate(root.selectedInstallation.lastPlayed))
                width: parent.width
                y: parent.height / 2
                color: Theme.colors.textDim
                font.family: Theme.font
                font.pixelSize: parent.height / 2.5
                elide: Text.ElideRight
            }
        }

        Item {

            visible: installationsModel.count > 1 && parent.width >= parent.height * 2

            Layout.fillHeight: true
            Layout.preferredWidth: parent.height

            Label {
                anchors.fill: parent
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignHCenter
                font.family: Theme.font
                font.pixelSize: 20
                text: ""
                color: Theme.colors.textDim
            }
        }
    }

    Popup {

        property NblGameInstallationsModel model: NblGameInstallationsModel {
            id: installationsModel
            gameId: root.gameId
            pageSize: 10
            onIsLoadingChanged: {
                if(!isLoading && root.selectedInstallation == null) {
                    root.selectedInstallation = ApplicationContext.getInstallationDetails(getId(0))
                }
            }
        }

        id: dropdown

        width: parent.width
        height: Math.min(160, (installationsModel.count * 48) + 16)

        y: parent.height + 8

        padding: 8
        background: Rectangle {
            color: Theme.colors.layer0
            border.width: 2
            border.color: Theme.colors.border
            radius: 8
        }

        contentItem: ListView {

            onAtYEndChanged: {
                if(atYEnd && !installationsModel.allLoaded && !installationsModel.isLoading) {
                    installationsModel.loadNextPage()
                }
            }

            model: installationsModel
            delegate: Item {

                id: installationItem
                width: parent.width
                height: 48

                required property var installationId
                required property string icon
                required property string resolvedTitle
                required property var lastPlayed

                Rectangle {
                    anchors.fill: parent
                    visible: mouseArea.containsMouse
                    color: Theme.colors.layer1
                    radius: 8
                }

                RowLayout {

                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Item {

                        Layout.fillHeight: true
                        Layout.preferredWidth: parent.height

                        NblImage {
                            id: dImageSource
                            anchors.fill: parent
                            anchors.verticalCenter: parent.verticalCenter
                            fillMode: Image.PreserveAspectFit
                            layer.enabled: true
                            visible: false
                            source: ApplicationContext.mediaUrl(installationItem.icon, "icon")
                        }

                        Item {
                            id: dMaskShape
                            anchors.fill: dImageSource
                            visible: false
                            layer.enabled: true
                            Rectangle {
                                anchors.fill: parent
                                radius: 8
                            }
                        }

                        MultiEffect {
                            anchors.fill: dImageSource
                            source: dImageSource
                            maskEnabled: true
                            maskSource: dMaskShape
                            maskThresholdMin: 0.5
                            maskSpreadAtMin: 1.0
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        visible: parent.width > parent.height * 2

                        Label {
                            text: installationItem.resolvedTitle
                            y: -(parent.height / 7)
                            width: parent.width
                            color: Theme.colors.text
                            font.family: Theme.font
                            font.pixelSize: parent.height / 2
                            font.weight: 500
                            elide: Text.ElideRight
                        }

                        Label {
                            text: "Last played: " + (installationItem.lastPlayed == 0 ? "Never" : Theme.formatDate(installationItem.lastPlayed))
                            width: parent.width
                            y: parent.height / 2
                            color: Theme.colors.textDim
                            font.family: Theme.font
                            font.pixelSize: parent.height / 2.5
                            elide: Text.ElideRight
                        }
                    }
                }
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.selectedInstallation = ApplicationContext.getInstallationDetails(installationItem.installationId)
                        dropdown.close()
                    }
                }
            }
        }
    }

    MouseArea {
        enabled: installationsModel.count > 1 && root.selectedInstallation != null
        anchors.fill: parent
        hoverEnabled: enabled
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: {
            dropdown.open()
        }
    }
}
