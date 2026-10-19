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

import Nebula

Image {

    id: root

    smooth: true
    mipmap: true
    asynchronous: true

    property bool usePlaceholder: true
    property color placeholderBg: Theme.colors.layer1
    property color placeholderText: Theme.colors.text

    Rectangle {

        id: placeholder
        anchors.fill: parent
        visible: root.usePlaceholder && root.status == Image.Loading

        color: root.placeholderBg
        
    }


}
