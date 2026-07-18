import QtQuick

Item {

    property bool modified: false

    function refresh() {
        refreshed()
        modified = false
    }

    function apply() {
        applied()
        modified = false
    }

    signal refreshed
    signal applied

}
