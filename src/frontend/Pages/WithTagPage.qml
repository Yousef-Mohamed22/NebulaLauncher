pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula.Controls

Item {

    id: root

    required property int tag
    required property string name
    required property color color

    property bool useIcons: false
    property bool showTitles: false

    signal gameClicked(int gameId)

    ScrollView {

        id: scrollView
        width: parent.width
        height: parent.height - 8
        y: 8

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        contentHeight: list.height

        Flickable {

            anchors.fill: parent
            interactive: true
            clip: true

            id: flickable

            Column {

                id: content
                width: parent.width

                Row {
                    x: 8
                    height: 32
                    spacing: 8

                    NblInlineIcon {
                        width: 20
                        height: 20
                        anchors.verticalCenter: parent.verticalCenter
                        icon: ""
                        color: Theme.colors.text
                    }

                    Label {
                        text: "Games tagged" 
                        font.family: Theme.font
                        font.pixelSize: 20
                        font.bold: true
                        color: Theme.colors.text
                    }

                    NblTag {
                        enabled: false
                        text: root.name
                        color: root.color
                        height: 28
                        textSize: 16
                        width: Math.max(implicitWidth, 98)
                    }
                }

                NblGameList {

                    id: list
                    width: parent.width

                    model: NblGameWithTagModel {
                        pageSize: 10
                        tag: root.tag
                    }
                    onLoadMore: {
                        model.loadNextPage()
                        allLoaded = model.allLoaded
                    }

                    multiLine: true
                    useIcons: root.useIcons
                    showTitles: root.showTitles

                    onItemClicked: id => {
                        root.gameClicked(id)
                    }
                }
            }
        }
    }

    function refresh() { }
}
