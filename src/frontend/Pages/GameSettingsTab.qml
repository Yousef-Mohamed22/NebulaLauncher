pragma ComponentBehavior: Bound

import QtQuick

import Nebula.Controls

Column {

    id: root
    spacing: 16

    NblTextBox {
        id: filterBox
        width: parent.width
        label: "Filter"
    }

    Repeater {

        model: []
        delegate: NblDropdown {

        }
    }
}

