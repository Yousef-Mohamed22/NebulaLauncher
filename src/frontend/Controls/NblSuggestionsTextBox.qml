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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic

import Nebula

NblTextBox {

    id: root

    function clearSuggestions() {
        popup.suggestionsModel.clear()
    }

    function pushSuggestion(str) {
        popup.suggestionsModel.append({ text: str })
        if(focus && !popup.opened) {
            popup.open()
        }
    }

    onPressed: {
        if(popup.suggestionsModel.count > 0 && !popup.opened) {
            popup.open()
        }
    }

    Keys.onReturnPressed: event => {
        if(popup.suggestionsModel.count > 0) {
            root.text = popup.suggestionsModel.get(listView.currentIndex).text
            root.textEdited()
            popup.close()
        }
        event.accepted = true
    }

    Keys.onEscapePressed: event => {
        popup.close()
        event.accepted = true
    }

    Keys.onDownPressed: event => {
        listView.incrementCurrentIndex()
        event.accepted = true
    }
    Keys.onUpPressed: event => {
        listView.decrementCurrentIndex()
        event.accepted = true
    }

    Popup {

        id: popup

        property ListModel suggestionsModel: ListModel {}

        y: root.height

        width: root.width
        height: Math.min(144, (suggestionsModel.count * 32) + 16)

        padding: 8

        background: Rectangle {
            radius: 8
            border.width: 2
            border.color: Theme.colors.border
            color: Theme.colors.layer0
        }
        contentItem: ListView {
            id: listView
            clip: true
            model: popup.suggestionsModel
            delegate: Item {

                width: root.width - 16
                height: 32

                required property int index
                required property string text

                Rectangle {
                    anchors.fill: parent
                    color: listView.currentIndex == parent.index ? Theme.colors.layer1 : "transparent"
                    radius: 4
                }
                Label {
                    anchors.fill: parent
                    anchors.margins: 4
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    elide: Text.ElideRight
                    font.family: Theme.font
                    font.pixelSize: 16
                    color: Theme.colors.text
                    text: parent.text
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onContainsMouseChanged: {
                        if(containsMouse) {
                            listView.currentIndex = parent.index
                        }
                    }
                    onClicked: {
                        root.text = parent.text
                        root.textEdited()
                        popup.close()
                    }
                }
            }
        }
    }
}
