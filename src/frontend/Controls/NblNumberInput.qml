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

NblTextBox {
    id: root

    property bool allowDecimal: false
    property var value: 0
    property var step: 1
    property var min: -100
    property var max: 100

    property var lastText: text

    inputMethodHints: allowDecimal ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly

    signal valueModified

    readonly property var effectveStep: allowDecimal ? step : Math.trunc(step)

    rightPadding: root.background.height
    text: "0"

    onValueChanged: {
        text = value
    }

    Item {
        width: root.background.height - 16
        height: root.background.height - 16
        y: root.background.y + 8
        x: root.background.x + root.background.width - width - 8

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label {
                    anchors.centerIn: parent
                    font.family: Theme.nerdFont
                    font.pixelSize: parent.height
                    text: ""
                    color: upMouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
                }
                MouseArea {
                    id: upMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.value = Math.min(root.max, root.value + root.effectveStep)
                        root.text = root.value
                        root.valueModified()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label {
                    anchors.centerIn: parent
                    font.family: Theme.nerdFont
                    font.pixelSize: parent.height
                    text: ""
                    color: downMouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
                }
                MouseArea {
                    id: downMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.value = Math.max(root.min, root.value - root.effectveStep)
                        root.text = root.value
                        root.valueModified()
                    }
                }
            }
        }

    }

    onCursorVisibleChanged: {
        if(value == null) return
        text = value
        lastText = text
    }

    onTextEdited: {
        if(text.length == 0 || (text.length == 1 && text == "-" && min < 0)) {
            value = 0
            lastText = ""
            return
        }

        if(!allowDecimal && text.includes('.')) {
            text = lastText
            return
        }

        let parsed = Number(text)
        if(isNaN(parsed) || (!allowDecimal && !Number.isInteger(parsed))) {
            text = lastText
            return
        }

        value = Math.max(root.min, Math.min(parsed, root.max))
        if(parsed != value) {
            text = value
        }
        lastText = text
        valueModified()
    }

}
