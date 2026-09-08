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
