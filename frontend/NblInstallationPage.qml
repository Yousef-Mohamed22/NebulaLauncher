import QtQuick
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

            implicitHeight: 240

            Image {
                id: banner
                source: "file://" + ApplicationContext.dataFolder + "/media/banner/" + root.details.banner
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

                //anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: logo
                    source: "file://" + ApplicationContext.dataFolder + "/media/logo/" + root.details.logo
                    anchors.centerIn: parent
                    width: parent.width - 16
                    height: parent.height - 16
                    fillMode: Image.PreserveAspectFit
                }

            }
        }

        RowLayout {

            Layout.fillWidth: true
            Layout.preferredHeight: 48
            Layout.leftMargin: 8

            NblLaunchButton {
                Layout.topMargin: -24
                details: root.details
            }

            // Spacer
            Item { Layout.fillWidth: true }

            NblIconButton {
                icon: ""
                onClicked: {
                    console.log("Clicked")
                }
            }
        }
    }
}
