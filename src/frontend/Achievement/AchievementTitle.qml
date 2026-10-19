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

import Nebula

NblBadgedLabel {
    id: root

    property string title: ""
    property bool isProgression: false
    property bool isWinCondition: false
    property bool isMissable: false

    text: title
    badges: _calculateBadges()

    function _calculateBadges() {
        let out = []
        if(isProgression) out.push({ icon: '', text: "Progression" })
        if(isWinCondition) out.push({ icon: '', text: "Win Condition" })
        if(isMissable) out.push({ icon: '', text: "Missable" })
        return out
    }
}
