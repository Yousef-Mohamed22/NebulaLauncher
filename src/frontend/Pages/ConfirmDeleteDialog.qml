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
import QtQuick.Controls

import Nebula.Controls

NblDialog {

    id: root
    property string text: "Hello"

    showApplyButton: false

    okText: "Confirm"
    okButtonColor: "red"

    targetWidth: 400
    targetHeight: 200

    Label {

        width: parent.width

        elide: Text.ElideRight

        text: "Are You Sure?"
        color: Theme.colors.text
        font.family: Theme.font
        font.pixelSize: 24
    }

    Label {

        width: parent.width
        anchors.centerIn: parent

        wrapMode: Text.WordWrap

        text: root.text
        color: Theme.colors.text
        font.family: Theme.font
        font.pixelSize: 16
    }

}
