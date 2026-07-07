pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {

    id: root

    property string icon: ""
    property color color: Theme.colors.text

    property bool colorize: true

    property string dbIcon: ""
    onDbIconChanged: {
        icon = getDbIconImage(dbIcon)
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
            fillMode: Image.PreserveAspectFit
            layer.enabled: true
            visible: !root.colorize
            smooth: true
            mipmap: true
        }

        MultiEffect {
            anchors.fill: imageSource
            source: imageSource
            colorization: 1.0
            colorizationColor: root.color
            visible: root.colorize
        }
    }

    function getDbIconImage(icon) {
        return root.useImage ? "file://" + ApplicationContext.dataFolder + "/media/inline_icon/" + icon : icon
    }

}
