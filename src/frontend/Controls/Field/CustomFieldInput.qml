import QtQuick

import Nebula

FieldInputDelegate {

    id: root
    implicitHeight: loader.item == null ? 0 : loader.item.height

    Loader {
        id: loader
        width: root.width
        active: root.field != null
        source: active ? Qt.url(`${ApplicationContext.pluginFolder}/${root.field.document}`) : null
    }
}
