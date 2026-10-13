pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Nebula

SettingsTab {

    id: root

    showApplyButton: false
    showCancelButton: false

    NblScrollView {

        width: parent.width + 16
        height: parent.height

        contentWidth: parent.width
        contentHeight: pluginsColumn.height

        clip: true

        Column {

            id: pluginsColumn
            width: root.width

            spacing: 8

            Repeater {
                id: pluginRepeater
                model: PluginModel {
                    id: pluginModel
                }

                delegate: Item {

                    id: pluginItem

                    required property var modelData
                    required property int index

                    width: root.width
                    height: 64

                    Rectangle {
                        anchors.fill: parent
                        color: Theme.colors.layer1
                        radius: 8
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            ColumnLayout {
                                anchors.fill: parent
                                Label {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    font.weight: 600
                                    color: Theme.colors.text
                                    text: pluginItem.modelData.name + " (v" + pluginItem.modelData.version + ")"
                                    elide: Text.ElideRight
                                    wrapMode: Text.NoWrap
                                    verticalAlignment: Qt.AlignTop
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: Theme.colors.textDim
                                    text: pluginItem.modelData.description
                                    elide: Text.ElideRight
                                    verticalAlignment: Qt.AlignBottom
                                }
                            }
                        }
                        Item {
                            Layout.fillHeight: true
                            Layout.preferredWidth: height

                            enabled: pluginItem.modelData.fieldCount > 0

                            Rectangle {
                                anchors.fill: parent
                                color: settingsArea.containsMouse ? Theme.colors.border : "transparent"
                                radius: 8
                            }
                            Label {
                                anchors.fill: parent
                                verticalAlignment: Qt.AlignVCenter
                                horizontalAlignment: Qt.AlignHCenter
                                font.family: Theme.nerdFont
                                font.pixelSize: 20
                                color: parent.enabled ? Theme.colors.text : Theme.colors.border
                                text: ""
                            }
                            MouseArea {
                                id: settingsArea
                                anchors.fill: parent
                                hoverEnabled: parent.enabled
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {

                                    let values = pluginItem.modelData.fieldValues

                                    let dialog = DialogManager.create("Plugin/PluginSettingsDialog.qml", {
                                        targetWidth: 700, 
                                        targetHeight: 700, 
                                        pluginName: pluginItem.modelData.name,
                                        fields: pluginModel.fields(pluginItem.index),
                                        values: values
                                    }) as NblDialog

                                    hoverEnabled = false
                                    dialog.closed.connect(function() {
                                        hoverEnabled = true
                                    })
                                    dialog.apply.connect(function() { 
                                        pluginModel.saveFields(dialog.save(), pluginItem.index)
                                    })
                                    dialog.open()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
