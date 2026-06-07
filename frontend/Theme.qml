pragma Singleton

import QtQuick

QtObject {

    property string font: _private.mainFont.name
    property string italicFont: _private.italicFont.name
    property string nerdFont: _private.nerdFont.name


    property var colors: QtObject {
        property color activeColor: "#278ce5"

        property color layer0: Qt.hsla(activeColor.hslHue, 0.2, 0.125, 1.0)
        property color layer1: Qt.lighter(layer0, 1.35)
        property color border: Qt.lighter(layer1, 1.35)
        property color text: "#ffffff"
        property color textDim: Qt.hsla(activeColor.hslHue, 0.1, 0.75, 1.0)
        property color accent: activeColor
    }

    property var _private: QtObject {

        property FontLoader mainFont: FontLoader {
            source: "qrc:/qt/qml/Nebula/fonts/NotoSansDisplay-VariableFont_wdth,wght.ttf"
        }
        property FontLoader italicFont: FontLoader {
            source: "qrc:/qt/qml/Nebula/fonts/NotoSansDisplay-VariableFont_wdth,wght.ttf"
        }
        property FontLoader nerdFont: FontLoader {
            source: "qrc:/qt/qml/Nebula/fonts/NotoSansMNerdFontPropo-Regular.ttf"
        }
    }

}
