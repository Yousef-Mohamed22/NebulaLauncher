import QtQuick

Item {

    id: root

    default property alias content: contentContainer.children

    property var popupWidth: 250
    property var popupHeight: 250

    visible: false

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.visible = false
        }
    }

    Item {
        id: contentContainer
        width: root.popupWidth
        height: root.popupHeight
    }

    function open(targetPos) {

        let maxPos = Qt.point(width - popupWidth, height - popupHeight)

        contentContainer.x = Math.min(maxPos.x, targetPos.x)
        contentContainer.y = Math.min(maxPos.y, targetPos.y)

        root.visible = true
    }

}
