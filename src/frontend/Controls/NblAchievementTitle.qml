pragma ComponentBehavior: Bound

import QtQuick

NblBadgedLabel {
    id: root

    property string title: ""
    property bool isProgression: false
    property bool isWinCondition: false
    property bool isMissable: false

    text: title
    badges: _calculateBadges()

    function _calculateBadges() {
        let out = []
        if(isProgression) out.push({ icon: '', text: "Progression" })
        if(isWinCondition) out.push({ icon: '', text: "Win Condition" })
        if(isMissable) out.push({ icon: '', text: "Missable" })
        return out
    }
}
