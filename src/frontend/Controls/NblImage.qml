import QtQuick

import Nebula

Image {

    id: root

    smooth: true
    mipmap: true
    asynchronous: true

    property bool usePlaceholder: true
    property color placeholderBg: Theme.colors.layer1
    property color placeholderText: Theme.colors.text

    Rectangle {

        id: placeholder
        anchors.fill: parent
        visible: root.usePlaceholder && root.status == Image.Loading

        color: root.placeholderBg
        
    }


}
