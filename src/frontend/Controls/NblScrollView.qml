import QtQuick
import QtQuick.Controls

import Nebula

ScrollView { 

    id: root

    readonly property bool atYEnd: flickable.atYEnd

    property alias contentY: flickable.contentY

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical: ScrollBar {
        parent: root

        x: root.width - 8
        height: root.height

        opacity: active && root.contentHeight > root.height ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }

        contentItem: Rectangle {
            implicitWidth: 6
            radius: 3
            color: Theme.colors.border
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: parent.clip
    }
}
