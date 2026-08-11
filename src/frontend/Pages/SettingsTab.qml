import QtQuick

Item {

    property bool showApplyButton: true
    property bool showCancelButton: true

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
