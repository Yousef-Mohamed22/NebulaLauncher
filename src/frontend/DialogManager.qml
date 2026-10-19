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


pragma Singleton

import QtQuick

QtObject {

    property Item dialogParent: null
    function create(path, params) {

        let dialogComponent = Qt.createComponent(path)
        if(dialogComponent.status != Component.Ready) {
            console.error("Unable to open dialog: ", dialogComponent.errorString())
            return null
        }

        let dialog = dialogComponent.createObject(dialogParent, params)

        let nblDialog = dialog as NblDialog
        if(nblDialog) {
            nblDialog.closed.connect(function() {
                nblDialog.destroy()
            })
        }

        return dialog
    }

    function openDeleteConfirmation(text, onApply) {
        let dialog = DialogManager.create("Pages/ConfirmDeleteDialog.qml", { 
            "text": text 
        }) as ConfirmDeleteDialog
        dialog.apply.connect(onApply)
        dialog.open()
    }

}
