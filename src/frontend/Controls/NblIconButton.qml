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

import Nebula

Item {

    id: root

    required property string icon

    signal clicked

    property color colorHovered: Theme.colors.text
    property color colorUnhovered: Theme.colors.textDim

    height: 48
    width: 48

    Rectangle {
        id: background
        anchors.fill: parent
        color: mouseArea.containsMouse ? Theme.colors.border : Theme.colors.layer1
        radius: 8
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.clicked()
        }
    }

    Label {
        text: root.icon
        anchors.centerIn: parent
        font.family: Theme.nerdFont
        font.pixelSize: 24
        color: mouseArea.containsMouse ? root.colorHovered : root.colorUnhovered   
    }

}
