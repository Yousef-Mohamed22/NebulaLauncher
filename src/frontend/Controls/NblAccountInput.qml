pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {

    id: root

    enum Type {
        BASIC = 0,
        OAUTH = 1,
        TOKEN = 2
    }

    property int type: NblAccountInput.Type.BASIC
    property bool cancelEnabled: true

    signal canceled
    signal loginPressed(var params)


    function clear() {
        usernameBox.clear()
        passwordBox.clear()
        tokenBox.clear()
    }

    RowLayout {

        anchors.fill: parent
        spacing: 8
        visible: root.type == NblAccountInput.Type.BASIC

        NblTextBox {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 180
            id: usernameBox
            label: "Username"
        }
        NblTextBox {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 180
            id: passwordBox
            label: "Password"
            echoMode: TextInput.Password
        }
        NblButton {
            Layout.fillWidth: true
            Layout.preferredWidth: 120
            Layout.preferredHeight: usernameBox.height - usernameBox.backgroundOffset
            Layout.alignment: Qt.AlignBottom
            text: "Login"
            inactiveColor: Theme.colors.accent
            activeColor: Theme.colors.accent
            textColor: Theme.colors.textOnAccent
            centered: true
            fontWeight: 500
            onClicked: {
                root.loginPressed({
                    username: usernameBox.text,
                    password: passwordBox.text
                })
            }
        }
        NblButton {
            Layout.fillWidth: true
            Layout.preferredWidth: 120
            Layout.preferredHeight: usernameBox.height - usernameBox.backgroundOffset
            Layout.alignment: Qt.AlignBottom
            text: "Cancel"
            enabled: root.cancelEnabled
            inactiveColor: Theme.colors.layer1
            textColor: root.cancelEnabled ? Theme.colors.text : Theme.colors.textDim
            centered: true
            fontWeight: 500
            onClicked: {
                root.canceled()
            }
        }
    }

    RowLayout {

        anchors.fill: parent
        spacing: 8
        visible: root.type == NblAccountInput.Type.OAUTH

        NblButton {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Login With Browser"
            inactiveColor: Theme.colors.accent
            activeColor: Theme.colors.accent
            textColor: Theme.colors.textOnAccent
            fontWeight: 500
            centered: true
            onClicked: {
                root.loginPressed({})
            }
        }
        NblButton {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Cancel"
            enabled: root.cancelEnabled
            inactiveColor: Theme.colors.layer1
            textColor: root.cancelEnabled ? Theme.colors.text : Theme.colors.textDim
            centered: true
            fontWeight: 500
            onClicked: {
                root.canceled()
            }
        }
    }

    RowLayout {

        anchors.fill: parent
        spacing: 8
        visible: root.type == NblAccountInput.Type.TOKEN

        NblTextBox {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 360
            id: tokenBox
            label: "Token / API Key"
        }
        NblButton {
            Layout.fillWidth: true
            Layout.preferredWidth: 120
            Layout.preferredHeight: tokenBox.height - tokenBox.backgroundOffset
            Layout.alignment: Qt.AlignBottom
            text: "Login"
            inactiveColor: Theme.colors.accent
            activeColor: Theme.colors.accent
            textColor: Theme.colors.textOnAccent
            centered: true
            fontWeight: 500
            onClicked: {
                root.loginPressed({
                    token: tokenBox.text
                })
            }
        }
        NblButton {
            Layout.fillWidth: true
            Layout.preferredWidth: 120
            Layout.preferredHeight: tokenBox.height - tokenBox.backgroundOffset
            Layout.alignment: Qt.AlignBottom
            text: "Cancel"
            enabled: root.cancelEnabled
            inactiveColor: Theme.colors.layer1
            textColor: root.cancelEnabled ? Theme.colors.text : Theme.colors.textDim
            centered: true
            fontWeight: 500
            onClicked: {
                root.canceled()
            }
        }
    }

}
