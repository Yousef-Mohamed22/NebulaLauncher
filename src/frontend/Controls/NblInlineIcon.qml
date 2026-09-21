pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {

    id: root

    property string icon: ""
    property color color: Theme.colors.text

    property bool colorize: true
    property int imageRadius: 0
    property bool smoothImage: true

    property string dbIcon: ""
    onDbIconChanged: {
        if(dbIcon.endsWith(".png") || dbIcon.endsWith(".svg")) {
            icon = getDbIconImage(dbIcon)
        } else {
            icon = dbIcon
        }
    }

    readonly property bool useImage: icon.endsWith(".png") || icon.endsWith(".svg")

    Label {
        id: text
        visible: !root.useImage
        anchors.centerIn: parent

        font.family: Theme.nerdFont
        font.pixelSize: Math.min(root.height, root.width)

        text: root.icon
        color: root.color
    }

    Item {
        id: image
        visible: root.useImage

        anchors.fill: parent

        Image {
            id: imageSource
            anchors.fill: parent
            source: root.useImage ? root.icon : ""
            smooth: root.smoothImage
            mipmap: root.smoothImage
            fillMode: Image.PreserveAspectFit
            layer.enabled: true
            layer.smooth: root.smoothImage
            visible: false
        }

        Item {
            id: maskShape
            anchors.fill: imageSource

            visible: false
            layer.enabled: true

            Rectangle {
                anchors.fill: parent
                radius: root.imageRadius
            }
        }

        MultiEffect {
            anchors.fill: imageSource
            source: imageSource
            maskEnabled: root.imageRadius > 0
            maskSource: maskShape
            colorization: root.colorize ? 1.0 : 0.0
            colorizationColor: root.color
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
        }
    }

    function getDbIconImage(icon) {
        return ApplicationContext.mediaUrl(icon, "inline_icon")
    }

}
