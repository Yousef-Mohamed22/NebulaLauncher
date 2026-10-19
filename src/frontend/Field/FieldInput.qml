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

Item {

    id: root

    property FieldRef field
    property bool wasModified

    signal modified

    implicitHeight: loader.item == null ? 0 : (loader.item as Item).height

    function save() {
        if(loader.item == null) return null
        wasModified = false
        return (loader.item as FieldInputDelegate).value
    }

    function load(value) {
        if(loader.item == null) {
            console.log("Not loaded!")
            return null
        }
        (loader.item as FieldInputDelegate).value = value
        wasModified = false
    }


    Loader {
        id: loader
        width: parent.width
    }

    Connections {
        target: loader.item
        function onValueChanged() {
            root.wasModified = true
            root.modified()
        }
    }

    onFieldChanged: {

        if(field == null) return

        let source = null
        switch(field.type) {
            case 0:
                source = "Delegate/StringFieldInput.qml"
                break
            case 1:
            case 2:
                source = "Delegate/NumberFieldInput.qml"
                break
            case 3:
                source = "Delegate/BoolFieldInput.qml"
                break
            case 4:
                source = "Delegate/EnumFieldInput.qml"
                break
            case 5:
                source = "Delegate/DateFieldInput.qml"
                break
            case 6:
                source = "Delegate/AccountFieldInput.qml"
                break
            case 7:
                source = "Delegate/ColorFieldInput.qml"
                break
            case 8:
                source = "Delegate/ListFieldInput.qml"
                break
            case 9:
                source = "Delegate/StructFieldInput.qml"
                break
            case 10:
                source = "Delegate/Action.qml"
                break
            case 11:
                source = "Delegate/ActionFieldInput.qml"
                break
        }

        if(source == null) {
            console.warn("Unknown field type: ", field.type)
            return
        }
        loader.setSource(source, {
            "field": field
        })
        loader.active = true
    }
}

