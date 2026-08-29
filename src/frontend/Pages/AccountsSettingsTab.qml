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

        width: parent.width
        height: parent.height

        id: flickable

        contentWidth: width
        contentHeight: providersColumn.height
        clip: true

        Column {
            width: flickable.width
            id: providersColumn
            spacing: 16


            Repeater {
                id: providerRepeater
                
                property int loggingInIndex: -1
                model: NblAccountProviderModel {
                    id: providerModel
                    onLoginFinished: (success, msg) => {
                        providerRepeater.itemAt(providerRepeater.loggingInIndex).model.refresh()
                        providerRepeater.loggingInIndex = -1
                    }
                    onLoginStatus: message => {
                        providerRepeater.itemAt(providerRepeater.loggingInIndex).pushStatus(message)
                    }
                }

                delegate: NblLabeledBorder {

                    id: providerItem
                    required property int index
                    required property var modelData

                    property NblAccountsModel model: providerRepeater.model.accounts(providerItem.index)
                    property bool adding: false

                    function pushStatus(text) {
                        loadingStatus.text = text
                    }

                    width: flickable.width
                    height: accountsColumn.height + topPadding + bottomPadding + backgroundOffset
                    label: modelData.name
                    padding: 16

                    Column {

                        id: accountsColumn
                        width: flickable.width - 32
                        spacing: 8

                        Repeater {
                            id: accountRepeater
                            model: providerItem.model
                            delegate: RowLayout {

                                id: accountItem
                                required property var modelData

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
                                    text: accountItem.modelData.name
                                    color: Theme.colors.text
                                    verticalAlignment: Qt.AlignVCenter
                                }

                                NblButton {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 80
                                    icon: ""
                                    text: "Sign Out"
                                    inactiveColor: Theme.colors.layer1
                                    textColor: enabled ? Theme.colors.text : Theme.colors.textDim
                                    centered: true
                                    fontWeight: 500
                                    enabled: providerRepeater.loggingInIndex == -1
                                    onClicked: {
                                        providerModel.logout(providerItem.index, accountItem.modelData.id)
                                        accountRepeater.model.refresh()
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

                        Item {

                            width: parent.width
                            height: 48

                            NblAccountInput {
                                visible: providerItem.adding
                                anchors.fill: parent
                                type: providerItem.modelData.accountType

                                onCanceled: {
                                    clear()
                                    providerItem.adding = false
                                }
                                onLoginPressed: args => {
                                    clear()
                                    providerItem.adding = false
                                    providerRepeater.loggingInIndex = providerItem.index
                                    if(providerItem.modelData.accountType == 0) {
                                        providerModel.loginBasic(providerItem.index, args.username, args.password)
                                    } else if(providerItem.modelData.accountType == 1) {
                                        providerModel.loginOAuth(providerItem.index)
                                    } else {
                                        providerModel.loginToken(providerItem.index, args.token)
                                    }
                                }
                            }

                            NblButton {
                                visible: !providerItem.adding && accountRepeater.count < providerItem.modelData.maxAccounts
                                width: Math.min(240, parent.width)
                                height: parent.height
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: " Add Account"
                                enabled: providerRepeater.loggingInIndex == -1
                                inactiveColor: Theme.colors.layer1
                                textColor: enabled ? Theme.colors.text : Theme.colors.textDim
                                centered: true
                                fontWeight: 500
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
}
