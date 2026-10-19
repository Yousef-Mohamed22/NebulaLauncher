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

Item {

    id: root
    property color color
    property int particleSize

    implicitWidth: 0
    implicitHeight: 0

    Component.onCompleted: sparkleEffect.snapPosition()

    onXChanged: sparkleEffect.snapPosition()
    onYChanged: sparkleEffect.snapPosition()

    ShaderEffect {
        id: sparkleEffect
        width: root.particleSize
        height: root.particleSize

        smooth: false

        property color starColor: root.color
        property real intensity: 0 
        fragmentShader: "qrc:/shaders/sparkle.frag.qsb" 

        layer.enabled: false

        function snapScenePoint(p) {
            const dpr = Screen.devicePixelRatio

            return Qt.point(
                Math.round(p.x * dpr) / dpr,
                Math.round(p.y * dpr) / dpr
            )
        }

        function snapPosition() {
            const posInScene = root.mapToItem(null, 0, 0)
            const snapped = snapScenePoint(posInScene)
            const snappedInParent = root.mapFromItem(null, snapped.x, snapped.y)

            x = snappedInParent.x - (width / 2)
            y = snappedInParent.y - (height / 2)
        }


    }
    SequentialAnimation {
        running: true
        loops: Animation.Infinite

        id: sparkleAnim
        property real multiplier: 1.0
        property real delay: 1.0

        ScriptAction {
            script: {
                sparkleAnim.multiplier = Math.random()
                sparkleAnim.delay = Math.random()
            }
        }
        PauseAnimation {
            duration: 450 + sparkleAnim.delay * 1200 
        }
        PropertyAnimation {
            target: sparkleEffect
            property: "intensity"
            from: 0
            to: 0.01 + sparkleAnim.multiplier * 0.025
            duration: 600 + sparkleAnim.multiplier * 200
        }
        PauseAnimation { duration: 300 + sparkleAnim.multiplier * 200 }
        PropertyAnimation {
            target: sparkleEffect
            property: "intensity"
            from: 0.01 + sparkleAnim.multiplier * 0.025
            to: 0
            duration: 500 + sparkleAnim.multiplier * 200
        }
    }
}

