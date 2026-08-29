import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Control {

    id: root

    property string icon: ""
    required property string text

    property color activeColor: Theme.colors.border
    property color inactiveColor: "transparent"
    property color textColor: Theme.colors.text
    property int fontWeight: 400 
    
    property int radius: 8
    property bool centered: false

    signal clicked

    padding: 8

    implicitHeight: 32 + topPadding + bottomPadding
    implicitWidth: textLabel.implicitWidth + (root.icon.length > 0 ? 8 + 24 : 0) + leftPadding + rightPadding

    Rectangle {
        id: background
        anchors.fill: parent
        color: mouseArea.containsMouse ? root.activeColor : root.inactiveColor
        radius: root.radius
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.enabled
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.clicked()
        }
    }

    RowLayout {

        x: root.leftPadding
        y: root.topPadding
        width: root.centered ? textLabel.implicitWidth + (root.icon.length > 0 ? 8 + 24 : 0) : (parent.width - root.leftPadding - root.rightPadding)

        spacing: 8

        Label {
            visible: root.icon.length > 0
            Layout.preferredWidth: 24
            text: root.icon
            font.family: Theme.nerdFont
            font.pixelSize: 24
            color: root.textColor
        }

        Label {
            id: textLabel
            Layout.fillWidth: true
            text: root.text
            font.family: Theme.font
            font.pixelSize: 16
            font.weight: root.fontWeight
            color: root.textColor
        }
    }
}
