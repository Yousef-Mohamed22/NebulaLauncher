pragma Singleton

import QtQuick

QtObject {

    property string font: _private.mainFont.name
    property string italicFont: _private.italicFont.name
    property string nerdFont: _private.nerdFont.name
    property string monospaceFont: _private.monoFont.name

    property string dateFormat: "yyyy/MM/dd"
    function formatDate(epochMilli) {
        return Qt.formatDate(new Date(epochMilli), dateFormat)
    }

    property var colors: QtObject {

        function calculateBrightness(color) {
            var r = color.r * 255
            var g = color.g * 255
            var b = color.b * 255
            var brightness = (r * 299 + g * 587 + b * 114) / 1000
            return brightness
        }

        function readableOn(color) {

            let adjustedSaturation = 0.0
            let adjustedLightness = 0.0

            if(color.hslLightness > 0.5) {
                adjustedSaturation = color.hslSaturation * 0.5
                adjustedLightness = 0.25
            } else {
                adjustedSaturation = color.hslSaturation * 1.5
                adjustedLightness = 0.85
            }

            return Qt.hsla(color.hslHue, color.hslSaturation, adjustedLightness)
        }

        function blackOrWhite(color) {
            return calculateBrightness(color) > 200 ? "#000000" : "#ffffff"
        }

        property bool lightMode: false
        property color activeColor: "#d658fc"

        property color accent: activeColor

        property color layer0: Qt.hsla(activeColor.hslHue, 0.3, lightMode ? 0.95 : 0.125, 1.0)
        property color layer1: Qt.hsla(activeColor.hslHue, 0.3, lightMode ? 0.9 : 0.169, 1.0)
        property color border: Qt.hsla(activeColor.hslHue, 0.3, lightMode ? 0.8 : 0.228, 1.0)
        property color text: lightMode ? "#000000" : "#ffffff"
        property color textDim: Qt.hsla(activeColor.hslHue, 0.3, lightMode ? 0.2 : 0.75, 1.0)
        property color textOnAccent: blackOrWhite(accent)

        property color shadow: lightMode ? "#99000000" : "#FF000000"
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
        property FontLoader monoFont: FontLoader {
            source: "qrc:/qt/qml/Nebula/fonts/NotoSansMono-VariableFont_wdth,wght.ttf"
        }
    }

}
