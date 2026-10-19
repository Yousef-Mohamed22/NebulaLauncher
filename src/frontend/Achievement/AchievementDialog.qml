// Copyright 2026 jwall64
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula

NblDialog {
    
    id: root

    required property int gameId

    property int setId: 0

    property var updated: new Set()

    Component.onCompleted: {
        if(setId != 0) setBox.currentValue = setId
        updateSet(setBox.currentValue)
    }
    function updateSet(setId) {
        if(!updated.has(setBox.currentValue)) {
            updated.add(setBox.currentValue)
            achievementUpdateRunner.update(root.gameId, setId)
        }
    }

    showApplyButton: false
    showOkButton: false

    cancelText: "Back"

    ColumnLayout {

        anchors.fill: parent
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            NblComboBox {
                Layout.fillWidth: true
                Layout.preferredWidth: 100
                id: setBox
                label: "Achivement Set"
                model: AchievementSetModel {
                    gameId: root.gameId
                    includeHidden: true
                }
                textRole: "name"
                valueRole: "id"
                onActivated: {
                    root.updateSet(currentValue)
                }
            } 
            NblComboBox {
                Layout.fillWidth: true
                Layout.preferredWidth: 50
                id: sortBox
                label: "Sort By"
                model: [ "Number", "Name", "Date Unlocked", "Progress", "Global Unlock Rate" ]
            }
            NblComboBox {
                Layout.fillWidth: true
                Layout.preferredWidth: 50
                id: sortOrderBox
                label: "Sort Order"
                model: [ "Ascending", "Descending" ]
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            AchievementUpdateRunner { 
                id: achievementUpdateRunner
                onUpdateComplete: ret => {
                    if(ret) {
                        achievementsModel.refresh()
                    }
                }
            }

            NblScrollView {

                id: scrollView
                width: parent.width
                height: parent.height
                clip: true
                contentHeight: contentColumn.height

                onAtYEndChanged: {
                    if(atYEnd && !achievementsModel.allLoaded) {
                        achievementsModel.loadNextPage()
                    }
                }

                Column {
                    width: parent.width - 24
                    id: contentColumn
                    spacing: 8
                    x: 12

                    Repeater {
                        id: achievementsRepeater
                        model: AchievementModel {
                            id: achievementsModel
                            setId: setBox.currentValue
                            sortBy: sortBox.currentIndex
                            sortOrder: sortOrderBox.currentIndex
                        }
                        delegate: Item {
                            id: contentItem
                            required property var modelData
                            required property int index

                            readonly property bool isProgression: modelData.flags & 1
                            readonly property bool isWinCondition: modelData.flags & 2
                            readonly property bool isSpoiler: modelData.dateUnlocked == 0 && (modelData.flags & 4)
                            readonly property bool isMissable: modelData.flags & 8
                            readonly property bool isRare: modelData.globalUnlockRate < 10.0
                            property bool isHidden: isSpoiler

                            readonly property string displayTitle: isHidden ? "Hidden Achievement" : modelData.title
                            readonly property string displayDescription: isHidden ? "Click to Show Spoiler" : modelData.description

                            width: contentColumn.width
                            height: 72

                            RowLayout {

                                anchors.fill: parent
                                spacing: 8

                                AchievementIcon {
                                    Layout.fillHeight: true
                                    Layout.preferredWidth: height
                                    Layout.margins: 4

                                    enabled: false
                                    title: contentItem.displayTitle
                                    icon: contentItem.modelData.dateUnlocked == 0 ? contentItem.modelData.lockedIcon : contentItem.modelData.icon
                                    borderColor: contentItem.modelData.borderColor
                                    unlocked: contentItem.modelData.dateUnlocked > 0
                                    hidden: contentItem.isHidden
                                    rare: contentItem.isRare
                                    borderThickness: 6
                                }

                                Item {
                                    Layout.fillHeight: true
                                    Layout.maximumWidth: contentColumn.width - rightLayout.implicitWidth - 16 - 64
                                    Layout.preferredWidth: contentColumn.width - rightLayout.implicitWidth - 16 - 64

                                    Column {
                                        width: parent.width
                                        anchors.verticalCenter: parent.verticalCenter
                                        AchievementTitle { 
                                            width: parent.width
                                            title: contentItem.displayTitle
                                            isProgression: !contentItem.isHidden && contentItem.isProgression
                                            isWinCondition: !contentItem.isHidden && contentItem.isWinCondition
                                            isMissable: !contentItem.isHidden && contentItem.isMissable
                                        }

                                        Label {
                                            id: descriptionLabel
                                            width: parent.width
                                            text: contentItem.displayDescription
                                            maximumLineCount: 3
                                            lineHeightMode: Text.ProportionalHeight
                                            lineHeight: lineCount > 2 ? 0.8 : 1.0
                                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                            font.family: Theme.font
                                            font.pixelSize: 14
                                            color: Theme.colors.textDim
                                            elide: Text.ElideRight

                                        }
                                    }
                                }

                                Item {
                                    id: rightLayout
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Layout.margins: 8
                                    implicitWidth: 200

                                    Column {
                                        width: parent.width
                                        anchors.verticalCenter: parent.verticalCenter

                                        Label {
                                            id: unlockText
                                            width: parent.width
                                            horizontalAlignment: Qt.AlignRight
                                            text: _getText()
                                            font.family: Theme.font
                                            font.pixelSize: 16
                                            color: Theme.colors.text

                                            function _getText() {

                                                let out = ""
                                                if(contentItem.modelData.dateUnlocked == 0) {
                                                    out = "Not yet unlocked" 
                                                    if(contentItem.modelData.denominator != 0 && !contentItem.isHidden) {
                                                        out += ` (${contentItem.modelData.numerator}/${contentItem.modelData.denominator})`
                                                    }
                                                } else {
                                                    out = "Unlocked on " + Theme.formatDate(contentItem.modelData.dateUnlocked)
                                                }
                                                return out
                                            }
                                        }

                                        Label {
                                            id: globalUnlockText
                                            width: parent.width
                                            horizontalAlignment: Qt.AlignRight
                                            text: "Global unlock rate: " + contentItem.modelData.globalUnlockRate.toFixed(1) + "%"
                                            font.family: Theme.font
                                            font.pixelSize: 14
                                            color: (contentItem.modelData.dateUnlocked > 0 && contentItem.isRare) ? contentItem.modelData.borderColor : Theme.colors.textDim 
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                enabled: contentItem.isSpoiler
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: {
                                    contentItem.isHidden = !contentItem.isHidden
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
