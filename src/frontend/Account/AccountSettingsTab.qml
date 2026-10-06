pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

SettingsTab {

    id: root

    showCancelButton: false
    showApplyButton: false

    onRefreshed: {
    }

    onApplied: {
    }

    NblScrollView {

        width: parent.width + 12
        height: parent.height

        id: flickable

        contentWidth: parent.width
        contentHeight: providersColumn.height
        clip: true

        Column {
            width: parent.width - 12
            id: providersColumn
            spacing: 16


            Repeater {
                id: providerRepeater
                
                property int loggingInIndex: -1
                model: AccountProviderModel {
                    id: providerModel
                }

                delegate: NblLabeledBorder {

                    id: providerItem
                    required property int index
                    required property var modelData

                    property bool adding: false

                    width: providersColumn.width
                    height: accountsColumn.height + topPadding + bottomPadding + backgroundOffset
                    label: modelData.name
                    padding: 16

                    Column {

                        id: accountsColumn
                        width: providersColumn.width - 32
                        spacing: 8

                        Repeater {
                            id: accountRepeater
                            model: providerModel.accountModel(providerItem.index)
                            delegate: RowLayout {

                                id: accountItem
                                required property var modelData

                                property bool expired: modelData.status == 2

                                spacing: 8

                                width: accountsColumn.width
                                height: 48

                                NblInlineIcon {
                                    Layout.fillHeight: true
                                    Layout.preferredWidth: height
                                    imageRadius: 8;
                                    icon: accountItem.modelData.icon
                                    smoothImage: !accountItem.modelData.iconNearestScaling
                                    color: Theme.colors.text
                                    colorize: false
                                }

                                Label {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 240
                                    font.pixelSize: 16
                                    font.weight: 500
                                    text: accountItem.modelData.username
                                    color: Theme.colors.text
                                    verticalAlignment: Qt.AlignVCenter
                                }
                                Label {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 16
                                    font.pixelSize: 16
                                    font.weight: 500
                                    text: ""
                                    color: "#d9a107"
                                    visible: accountItem.expired
                                    verticalAlignment: Qt.AlignVCenter

                                    NblToolTip {
                                        delay: 0
                                        text: "Expired"
                                        visible: expiredHoverArea.containsMouse
                                    }

                                    MouseArea {
                                        id: expiredHoverArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }
                                }

                                NblButton {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 80
                                    icon: ""
                                    text: "Refresh"
                                    inactiveColor: Theme.colors.layer1
                                    textColor: enabled ? Theme.colors.text : Theme.colors.textDim
                                    centered: true
                                    font.weight: 500
                                    visible: accountItem.expired
                                    enabled: providerRepeater.loggingInIndex == -1
                                    onClicked: {
                                        let fields = accountItem.modelData.fieldValues
                                        accountRepeater.model.logout(accountItem.modelData.id)
                                        accountFields.load(fields)
                                        providerItem.adding = true
                                    }
                                }

                                NblButton {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 80
                                    icon: ""
                                    text: "Sign Out"
                                    inactiveColor: Theme.colors.layer1
                                    textColor: enabled ? Theme.colors.text : Theme.colors.textDim
                                    centered: true
                                    font.weight: 500
                                    enabled: providerRepeater.loggingInIndex == -1
                                    onClicked: {
                                        accountRepeater.model.logout(accountItem.modelData.id)
                                    }
                                }
                            }
                        }

                        RowLayout {

                            id: loadingAccountItem

                            visible: providerRepeater.loggingInIndex == providerItem.index
                            spacing: 8

                            width: accountsColumn.width
                            height: 48

                            Connections {
                                target: accountRepeater.model
                                function onLoginStatus(message) {
                                    loadingStatus.text = message
                                }
                                function onLoginFinished(success, message) {
                                    providerRepeater.loggingInIndex = -1
                                    accountRepeater.model.refresh()
                                }
                            }

                            NblInlineIcon {
                                Layout.fillHeight: true
                                Layout.preferredWidth: height
                                imageRadius: 8;
                                icon: ''
                                color: Theme.colors.textDim
                                colorize: false
                            }

                            Label {
                                Layout.fillHeight: true
                                Layout.preferredWidth: implicitWidth
                                font.pixelSize: 16
                                font.weight: 500
                                text: "Logging in..."
                                color: Theme.colors.textDim
                                verticalAlignment: Qt.AlignVCenter
                            }

                            Label {
                                id: loadingStatus
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                font.pixelSize: 16
                                font.weight: 500
                                elide: Text.ElideRight
                                text: ""
                                color: Theme.colors.textDim
                                verticalAlignment: Qt.AlignVCenter
                                horizontalAlignment: Qt.AlignRight
                            }
                        }

                        RowLayout {
                            id: fieldsLayout
                            width: parent.width
                            spacing: 8
                            visible: providerItem.adding

                            NblFieldInputRepeater {
                                id: accountFields
                                fields: accountRepeater.model.fields
                                itemWidth: ((parent.width - 96 - 96 - 8) / count) - 8
                            }

                            Row {
                                Layout.preferredHeight: 48
                                Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                                spacing: 8
                                NblButton {
                                    centered: true
                                    width: 96
                                    activeColor: Theme.colors.border
                                    inactiveColor: Theme.colors.border
                                    text: "Cancel"
                                    onClicked: {
                                        providerItem.adding = false
                                    }
                                }
                                NblButton {
                                    centered: true
                                    activeColor: Theme.colors.accent
                                    inactiveColor: Theme.colors.accent
                                    textColor: Theme.colors.textOnAccent
                                    text: providerItem.modelData.loginText
                                    onClicked: {
                                        providerItem.adding = false
                                        if(accountRepeater.model.login(accountFields.save()) == 0) {
                                            providerRepeater.loggingInIndex = providerItem.index
                                        }
                                    }
                                }
                            }
                        }

                        NblButton {
                            visible: !providerItem.adding && accountRepeater.count < providerItem.modelData.maxAccounts
                            width: Math.min(240, parent.width)
                            height: 48
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: " Add Account"
                            enabled: providerRepeater.loggingInIndex == -1
                            inactiveColor: Theme.colors.layer1
                            textColor: enabled ? Theme.colors.text : Theme.colors.textDim
                            centered: true
                            font.weight: 500
                            onClicked: {
                                providerItem.adding = true
                            }
                        }
                    }
                }
            }
        }
    }
}
