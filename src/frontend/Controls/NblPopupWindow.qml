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
