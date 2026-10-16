import QtQuick

import Nebula

Item {

    id: root

    property string source: ""
    property int duration: 200
    property int initialDuration: 50
    property var fillMode: Image.PreserveAspectFit

    onSourceChanged: {
        if (backImage.source === root.source) return;
        backImage.source = root.source
    }

    Component.onCompleted: {
        frontImage.source = root.source
    }

    NblImage {
        id: backImage
        anchors.fill: parent
        opacity: 0.0
        fillMode: root.fillMode

        onStatusChanged: {
            if(source === frontImage.source) return
            if (status === Image.Ready) {
                crossfadeAnimation.start()
            }
        }
    }

    NblImage {
        id: frontImage
        anchors.fill: parent
        opacity: 0.0
        fillMode: root.fillMode
        onStatusChanged: {
            if(crossfadeInAnimation.initialFadeIn) return
            if (status === Image.Ready) {
                crossfadeInAnimation.start()
            }
        }
    }

    SequentialAnimation {
        id: crossfadeAnimation
        
        ParallelAnimation {
            NumberAnimation { target: frontImage; property: "opacity"; to: 0.0; duration: root.duration; easing.type: Easing.InOutQuad }
            NumberAnimation { target: backImage; property: "opacity"; to: 1.0; duration: root.duration; easing.type: Easing.InOutQuad }
        }
        ScriptAction {
            script: {
                frontImage.source = backImage.source
                frontImage.opacity = 1.0
                backImage.opacity = 0.0
            }
        }
    }
    
    SequentialAnimation {
        property bool initialFadeIn: false

        id: crossfadeInAnimation

        NumberAnimation { 
            target: frontImage; property: "opacity"; to: 1.0; duration: root.initialDuration; easing.type: Easing.InOutQuad 
        }
        ScriptAction {
            script: {
                crossfadeInAnimation.initialFadeIn = true;
            }
        }
    }
}
