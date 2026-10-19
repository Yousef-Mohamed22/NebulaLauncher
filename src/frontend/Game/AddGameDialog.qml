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

NblDialog {

    id: root
    showApplyButton: false

    onApply: {
        let id = ApplicationContext.addGame(settings.encode())
        if(id > 0) {
            gameAdded(id)
        }
    }

    signal gameAdded(var id)

    NblScrollView {

        id: scrollView
        width: parent.width + 12
        height: parent.height
        contentHeight: settings.height
        contentWidth: width - 12
        clip: true

        GameSettings {
            id: settings
            width: scrollView.contentWidth
        }
    }

    function load(meta) {
        settings.load(meta)
    }
}
