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
