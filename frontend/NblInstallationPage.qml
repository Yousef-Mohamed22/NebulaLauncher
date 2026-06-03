import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {

    id: root
    required property NblInstallationDetails details

    ColumnLayout {

        width: parent.width

        spacing: 8

        Item {

            Layout.fillWidth: true
            Layout.preferredHeight: 240

            height: 240

            Image {
                id: banner
                source: "file://" + ApplicationContext.dataFolder + "/media/banner/" + details.banner
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

            Item {

                height: parent.height
                width: 256

                Image {
                    id: logo
                    source: "file://" + ApplicationContext.dataFolder + "/media/logo/" + details.logo
                    anchors.centerIn: parent
                    width: parent.width - 16
                    height: parent.height - 16
                    fillMode: Image.PreserveAspectFit
                }

            }
        }

        RowLayout {

            Layout.fillWidth: true
            Layout.preferredHeight: 64

            Rectangle {

                Layout.fillHeight: true
                width: 192

                color: Theme.colors.accent
                radius: 8

                Row {
                    spacing: 8
                    anchors.centerIn: parent

                    Label {
                        text: ""
                        font.family: Theme.nerdFont
                        font.pixelSize: 28
                        color: Theme.colors.text
                    }
                    Label {
                        text: "LAUNCH"
                        font.family: Theme.font
                        font.pixelSize: 28
                        font.bold: true
                        color: Theme.colors.text
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: {
                        ApplicationContext.launchInstallation(details.installationId)
                    }
                }
            }
        }
    }
}
