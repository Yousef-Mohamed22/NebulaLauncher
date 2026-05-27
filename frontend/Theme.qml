pragma Singleton

import QtQuick
import QtQuick.Controls

QtObject {

    property string font: _private.mainFont.name
    property string italicFont: _private.italicFont.name
    property string nerdFont: _private.nerdFont.name

    property QtObject colors: QtObject {
        property color layer0: "#140c16"
        property color layer1: "#2d212d"
        property color border: "#3e2c3e"
        property color text: "#ffffff"
    }

    property QtObject _private: QtObject {
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
