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
