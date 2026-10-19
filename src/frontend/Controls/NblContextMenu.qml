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

Popup { 
    id: contextPopup
    visible: false

    implicitWidth: 200

    x: parent.width + 16
    y: parent.height - height

    margins: 0
    padding: 2

    background: Rectangle {
        color: Theme.colors.layer0
        border.width: 2
        border.color: Theme.colors.border
        radius: 8
    }
}
