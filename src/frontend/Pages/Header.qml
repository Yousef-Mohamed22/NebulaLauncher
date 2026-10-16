import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula

Item {

    id: root

    property Sidebar sidebar


    signal searchRequested(string query)
    signal randomRequested

    function focusSearchBar() {
        searchBar.forceActiveFocus()
    }


    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.colors.layer1
        border.width: 2
        border.color: Theme.colors.border
        radius: 8
    }

    RowLayout {
        anchors.fill: background
        anchors.margins: 8

        Row {

            Layout.fillHeight: true
            spacing: 8

            Rectangle {
                color: "transparent"

                height: parent.height
                width: height

                Label {
                    text: "󰍜"
                    font.family: Theme.nerdFont
                    font.pixelSize: 28
                    color: Theme.colors.text
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.sidebar.collapsed = !root.sidebar.collapsed
                    }
                }
            }

            Item {

                height: parent.height
                width: height
                visible: root.width > 600

                NblImage {
                    id: logoSource
                    anchors.fill: parent
                    anchors.margins: 4
                    source: "qrc:/qt/qml/Nebula/images/Nebula.png"
                    usePlaceholder: false
                    layer.enabled: true
                    visible: false
                }
                ShaderEffect {
                    anchors.fill: logoSource
                    property variant source: logoSource
                    property real uHue: Theme.colors.accent.hsvHue * (2 * Math.PI)
                    fragmentShader: "qrc:/shaders/hueShift.frag.qsb"
                }
            }

            Label {
                text: "Nebula"
                height: parent.height
                visible: root.width > 600
                verticalAlignment: Qt.AlignVCenter
                font.family: Theme.font
                font.bold: true
                font.pixelSize: 24
                color: Theme.colors.text
            }

        }

        GameSearchBar {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: 420

            id: searchBar

            onSearchRequested: (query) => {
                root.searchRequested(query)
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight

            implicitWidth: height

            property int previousRoll: 4

            property list<string> rolls: [ '󰇊', '󰇋', '󰇌', '󰇍', '󰇎', '󰇏' ]

            Label {
                id: randomLabel
                text: parent.rolls[parent.previousRoll] //  
                font.family: Theme.nerdFont
                font.pixelSize: 24
                color: Theme.colors.text
                anchors.centerIn: parent
            }

            NblToolTip {
                text: "Opens the page for a random installed game in your library"
                delay: 1000
                visible: randomMouseArea.containsMouse
            }

            Timer {
                interval: 200
                repeat: true
                triggeredOnStart: true
                running: randomMouseArea.containsMouse
                onTriggered: {
                    let roll = Math.floor(Math.random() * 5) + 1
                    parent.previousRoll = (parent.previousRoll + roll) % 6
                }
            }

            MouseArea {
                id: randomMouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    root.randomRequested()
                }
            }
        }
    }
}
