import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

NblTextBox {
    id: root

    property bool allowDecimal: false
    property var value: 0
    property var step: 1
    property var min: -100
    property var max: 100

    inputMethodHints: allowDecimal ? Qt.ImhFormattedNumbersOnly : Qt.ImhDigitsOnly

    signal valueModified

    readonly property var effectveStep: allowDecimal ? step : Math.trunc(step)

    rightPadding: root.background.height
    text: "0"

    Item {
        width: root.background.height - 16
        height: root.background.height - 16
        y: root.background.y + 8
        x: root.background.x + root.background.width - width - 8

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label {
                    anchors.centerIn: parent
                    font.family: Theme.nerdFont
                    font.pixelSize: parent.height
                    text: ""
                    color: upMouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
                }
                MouseArea {
                    id: upMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.value = Math.min(root.max, root.value + root.effectveStep)
                        root.text = root.value
                        root.valueModified()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label {
                    anchors.centerIn: parent
                    font.family: Theme.nerdFont
                    font.pixelSize: parent.height
                    text: ""
                    color: downMouseArea.containsMouse ? Theme.colors.text : Theme.colors.textDim
                }
                MouseArea {
                    id: downMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.value = Math.max(root.min, root.value - root.effectveStep)
                        root.text = root.value
                        root.valueModified()
                    }
                }
            }
        }

    }

    onTextEdited: {
        if(text.length == 0 || (text.length == 1 && text == "-" && min < 0)) {
            value = 0
            return
        }
        let parsed = allowDecimal ? parseFloat(text) : parseInt(text)
        if(isNaN(parsed)) {
            parsed = 0
        }

        value = Math.max(root.min, Math.min(parsed, root.max))

        // TODO: Allow entering decimals
        text = value

        valueModified()
    }

}
