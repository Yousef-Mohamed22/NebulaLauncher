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

CheckBox {

    id: root

    font.family: Theme.font
    font.pixelSize: 16

    implicitHeight: indicator.height
    leftPadding: 0

    property alias elide: contentText.elide

    indicator: Rectangle {
        implicitWidth: 32
        implicitHeight: 32        
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: 8

        color: Theme.colors.layer0
        border.width: 2
        border.color: root.checked ? Theme.colors.accent : Theme.colors.border

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            color: Theme.colors.accent
            radius: 4
            visible: root.checked
        }
    }

    contentItem: Text {
        id: contentText
        text: root.text
        font: root.font
        color: root.checked ? Theme.colors.text : Theme.colors.textDim
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
    }

}
