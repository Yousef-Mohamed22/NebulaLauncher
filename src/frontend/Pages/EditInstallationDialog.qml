pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

NblDialog {

    id: root
    property int installationId: 0
    property int gameId: 0

    padding: 0
    navigationPadding: 16

    onInstallationIdChanged: {
        installationSettingsTab.refresh()
        gameSettingsTab.refresh()
    }

    onApply: {
        navigation.currentTab.apply()
    }

    ColumnLayout {

        anchors.fill: parent

        Item {
            id: navigation

            property SettingsTab currentTab: gameSettingsTab

            Layout.fillWidth: true
            Layout.preferredHeight: 48

            RowLayout {
                width: Math.min(parent.width, 316)
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 0

                Repeater {
                    model: [ 
                        { name: "Game", tab: gameSettingsTab },
                        { name: "Installation", tab: installationSettingsTab }
                    ]

                    delegate: Item {

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        required property var modelData

                        readonly property bool isActive: navigation.currentTab == modelData.tab
                        readonly property bool isHighlighted: isActive || tabArea.containsMouse

                        Rectangle {
                            anchors.fill: parent
                            color: parent.isActive
                                    ? Theme.colors.border
                                    : parent.isHighlighted ? Theme.colors.layer1 : "transparent"
                        }

                        Label {
                            anchors.centerIn: parent
                            text: parent.modelData.name
                            font.family: Theme.font
                            font.pixelSize: 16
                            color: parent.isHighlighted ? Theme.colors.text : Theme.colors.textDim
                        }

                        MouseArea {
                            id: tabArea
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            anchors.fill: parent
                            onClicked: {
                                if(parent.modelData.tab == navigation.currentTab) return
                                if(navigation.currentTab.modified) {
                                    ApplicationContext.pushNotification({
                                        type: "error",
                                        title: "Cannot Change Tabs",
                                        message: "Please apply your changes first."
                                    })
                                    return
                                }

                                navigation.currentTab.modified = false
                                navigation.currentTab.visible = false

                                navigation.currentTab = parent.modelData.tab
                                navigation.currentTab.refresh()
                                navigation.currentTab.visible = true
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width - 64
                height: 2
                x: 32
                y: parent.height - 2
                color: Theme.colors.border
            }
        }

        Item {

            Layout.fillWidth: true
            Layout.fillHeight: true

            SettingsTab {

                id: gameSettingsTab

                anchors.fill: parent

                modified: gameSettings.modified

                onRefreshed: {
                    gameSettings.gameId = root.gameId
                }

                onApplied: {

                    let settings = gameSettings.encode()
                    let rc = ApplicationContext.updateGame(settings)
                    if(rc != 0) {
                        ApplicationContext.pushNotification({
                            type: "error",
                            title: "Failed to Update a Game",
                            message: "An error occurred while updating the game \"" + settings.title + "\""
                        })
                        console.log("An error occurred while updating a game!", rc)
                    }

                }

                ScrollView {

                    width: parent.width
                    height: parent.height - 16
                    y: 16

                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    contentWidth: width - 48
                    contentHeight: gameSettings.height

                    Flickable {

                        anchors.fill: parent
                        clip: true

                        NblGameSettings {
                            id: gameSettings
                            width: parent.width
                            x: 24
                        }
                    }
                }
            }

            SettingsTab {

                id: installationSettingsTab

                anchors.fill: parent
                visible: false

                property int displayId: 0

                onRefreshed: {

                    let details = ApplicationContext.getInstallationDetails(root.installationId)

                    root.gameId = details.gameId
                    gameNameBox.text = details.gameTitle

                    commandTypeBox.currentValue = details.type
                    fieldList.loadFromJson(details.command)

                    installationTitleField.text = details.title
                    installationTitleField.cursorPosition = 0

                    installationDescriptionField.text = details.description
                    installationDescriptionField.cursorPosition = 0

                    installationTags.initialTags = ApplicationContext.getInstallationTags(root.installationId)

                    displayId = details.displayId
                    overrideBox.checked = displayId != 0

                    if(details.poster) installationPosterPicker.imageSource = details.posterPath()  
                    if(details.icon) installationIconPicker.imageSource = details.iconPath() 
                    if(details.logo) installationLogoPicker.imageSource = details.logoPath() 
                    if(details.banner) installationBannerPicker.imageSource = details.bannerPath()

                    installationLogoPositionBox.currentIndex = details.logoPosition
                }

                onApplied: {

                    if(root.installationId == 0) return

                    let command = fieldList.saveToJson()

                    let poster = null
                    let icon = null
                    let logo = null
                    let banner = null
                    if(overrideBox.checked) {
                        poster = installationPosterPicker.imageSource
                        icon = installationIconPicker.imageSource
                        logo = installationLogoPicker.imageSource
                        banner = installationBannerPicker.imageSource
                    }

                    let params = {
                        game: root.gameId,
                        type: commandTypeBox.currentValue,
                        command: command,
                        displayOverride: overrideBox.checked,
                        displayId: displayId,
                        title: installationTitleField.text,
                        description: installationDescriptionField.text,
                        banner: banner,
                        poster: poster,
                        icon: icon,
                        logo: logo,
                        logoPosition: installationLogoPositionBox.currentIndex,
                        addedInstallationTags: installationTags.getAddedTags(),
                        removedInstallationTags: installationTags.getRemovedTags()
                    }

                    ApplicationContext.updateInstallation(root.installationId, params)
                }

                ScrollView {

                    width: parent.width
                    height: parent.height - 16
                    y: 16

                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    contentWidth: width - 48
                    contentHeight: controls.height

                    Flickable {

                        anchors.fill: parent
                        clip: true

                        Column {

                            id: controls
                            x: 24
                            width: parent.width
                            spacing: 16

                            NblTextBox {
                                id: gameNameBox
                                width: parent.width
                                enabled: false
                                label: "Game"
                            }

                            NblComboBox {
                                id: commandTypeBox
                                label: "Type"
                                width: parent.width
                                model: NblInstallationTypeModel { }
                                textRole: "name"
                                valueRole: "id"

                                onActivated: {
                                    installationSettingsTab.modified = true
                                }
                            }

                            NblFieldList { 
                                id: fieldList
                                typeId: commandTypeBox.currentValue

                                onModified: {
                                    installationSettingsTab.modified = true
                                }
                            }

                            NblTextBox {
                                id: installationTitleField
                                label: "Installation Title"
                                placeholderText: "Optional"
                                width: parent.width

                                onTextEdited: {
                                    installationSettingsTab.modified = true
                                }
                            }

                            NblTextBox {
                                id: installationDescriptionField
                                label: "Installation Description"
                                placeholderText: "Optional"
                                width: parent.width

                                onTextEdited: {
                                    installationSettingsTab.modified = true
                                }
                            }

                            NblTagList {
                                id: installationTags
                                label: "Installation Tags"
                                width: parent.width

                                onModified: {
                                    installationSettingsTab.modified = true
                                }
                            }
                            NblCheckBox {

                                id: overrideBox
                                text: "Override display assets for this installation"

                                onToggled: {
                                    installationSettingsTab.modified = true
                                }
                            }


                            Column {
                                visible: overrideBox.checked

                                width: parent.width
                                spacing: 8

                                RowLayout {

                                    width: parent.width
                                    height: 200

                                    NblImagePicker {
                                        id: installationPosterPicker
                                        label: "Poster"
                                        implicitHeight: parent.height
                                        Layout.fillWidth: true
                                        aspectRatio: 2.0 / 3.0
                                        onImageSourceChanged: {
                                            installationSettingsTab.modified = true
                                        }
                                    }
                                    NblImagePicker {
                                        id: installationIconPicker
                                        label: "Icon"
                                        implicitHeight: parent.height
                                        Layout.fillWidth: true
                                        onImageSourceChanged: {
                                            installationSettingsTab.modified = true
                                        }
                                    }
                                    NblImagePicker {
                                        id: installationLogoPicker
                                        label: "Logo"
                                        implicitHeight: parent.height
                                        Layout.fillWidth: true
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }

                                NblImagePicker {
                                    id: installationBannerPicker
                                    label: "Banner"
                                    height: 200
                                    width: parent.width
                                    aspectRatio: 96.0 / 31.0

                                    onImageSourceChanged: {
                                        installationSettingsTab.modified = true
                                    }
                                }

                                NblComboBox {
                                    id: installationLogoPositionBox
                                    label: "Logo Position"
                                    width: parent.width
                                    model: [ "Left", "Center", "Right", "Hidden" ]
                                    onActivated: {
                                        installationSettingsTab.modified = true
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
