// Copyright 2026 jwall64
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula

Control {

    id: root

    property string icon: ""
    required property string text

    property color activeColor: Theme.colors.border
    property color inactiveColor: "transparent"
    property color textColor: Theme.colors.text
    
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

        x: root.centered ? (root.width - width) / 2 : root.leftPadding 
        y: root.centered ? (root.height - height) / 2 : root.topPadding

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
            color: root.textColor
        }
    }
}
