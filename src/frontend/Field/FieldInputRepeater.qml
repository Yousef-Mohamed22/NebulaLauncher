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

Repeater {

    id: root

    required property FieldList fields
    property bool wasModified: false

    signal modified

    property int itemWidth: width

    model: root.fields ? root.fields.visibleCount : 0
    delegate: FieldInput {
        required property var index
        field: root.fields ? root.fields.visibleItemAt(index) : null
        width: root.itemWidth

        onModified: {
            root.wasModified = true
            root.modified()
        }
    }

    function load(encodedFields) {
        if(encodedFields) {
            for (const [key, value] of Object.entries(encodedFields)) {
                for(let i = 0 ; i < root.count ; i++) {
                    let item = root.itemAt(i) as FieldInput
                    if(item.field.key == key) {
                        item.load(value)
                    }
                }
            }
        } else {
            for(let i = 0 ; i < root.count ; i++) {
                let item = root.itemAt(i) as FieldInput
                item.load(null)
            }
        }
        wasModified = false
    }

    function save() {
        let fields = {}
        for(let i = 0 ; i < root.count ; i++) {
            let field = root.itemAt(i) as FieldInput
            fields[field.field.key] = field.save()
        }
        wasModified = false
        return fields
    }
}
