pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

NblDialog {

    id: root
    property string text: "Hello"

    showApplyButton: false

    okText: "Confirm"
    okButtonColor: "red"

    targetWidth: 400
    targetHeight: 200

    Label {

        width: parent.width

        elide: Text.ElideRight

        text: "Are You Sure?"
        color: Theme.colors.text
        font.family: Theme.font
        font.pixelSize: 24
    }

    Label {

        width: parent.width

        wrapMode: Text.WordWrap

        text: root.text
        color: Theme.colors.text
        font.family: Theme.font
        font.pixelSize: 16
    }

}
