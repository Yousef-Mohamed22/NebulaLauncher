pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Nebula

NblListInput {

    required property var field
    required property int fieldIndex
    required property string typeId
    property NblFieldModel fieldModel

    readonly property var QModelIndex: fieldModel.index(fieldIndex, 1)

    signal modified

    createNew: function(index) {
        return {}
    }

    delegate: NblFieldInput {

        

    }

}
