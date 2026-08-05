pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

NblDialog {

    id: root
    property int installationId: 0
    property int gameId: 0
    property bool startOnInstallation: false

    padding: 8
    navigationPadding: 8

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

            property SettingsTab currentTab: null

            Component.onCompleted: {
                if(root.startOnInstallation) {
                    currentTab = installationSettingsTab
                } else {
                    currentTab = gameSettingsTab
                }
            }

            onCurrentTabChanged: {
                currentTab.refresh()
            }

            Layout.fillWidth: true
            Layout.preferredHeight: 40

            RowLayout {
                width: Math.min(parent.width, 316)
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 0

                Repeater {
                    model: [ 
                        { icon: "󰊗", name: "Game", tab: gameSettingsTab },
                        { icon: "", name: "Installation", tab: installationSettingsTab }
                    ]

                    delegate: Item {

                        id: tabItem

                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        required property var modelData

                        readonly property bool isActive: navigation.currentTab == modelData.tab
                        readonly property bool isHighlighted: isActive || tabArea.containsMouse
 
                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 8

                            property color color: 
                                parent.isActive 
                                    ? Theme.colors.accent 
                                    : parent.isHighlighted 
                                        ? Theme.colors.text 
                                        : Theme.colors.textDim

                            Label {
                                text: tabItem.modelData.icon
                                font.family: Theme.nerdFont
                                font.pixelSize: 16
                                color: parent.color
                            }

                            Label {
                                text: tabItem.modelData.name
                                font.family: Theme.font
                                font.pixelSize: 16
                                color: parent.color
                            }
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 96
                            height: 4
                            radius: 2
                            color: Theme.colors.accent
                            visible: parent.isActive
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
                                navigation.currentTab.visible = true
                            }
                        }
                    }
                }
            }
        }

        Item {

            Layout.fillWidth: true
            Layout.fillHeight: true

            SettingsTab {

                id: gameSettingsTab
                visible: !root.startOnInstallation
                anchors.fill: parent

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

                            onModifiedChanged: {
                                gameSettingsTab.modified = modified
                            }
                        }
                    }
                }
            }

            SettingsTab {

                id: installationSettingsTab

                anchors.fill: parent
                visible: root.startOnInstallation

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

                    if(details.poster) installationPosterPicker.imageSource = details.resolvedPosterUrl
                    if(details.icon) installationIconPicker.imageSource = details.resolvedIconUrl
                    if(details.logo) installationLogoPicker.imageSource = details.resolvedLogoUrl 
                    if(details.banner) installationBannerPicker.imageSource = details.resolvedBannerUrl

                    installationLogoPositionBox.currentIndex = details.logoPosition
                }

                onApplied: {

                    if(root.installationId == 0) return
                    let command = fieldList.saveToJson()

                    let params = {
                        game: root.gameId,
                        type: commandTypeBox.currentValue,
                        command: command,
                        title: installationTitleField.text,
                        description: installationDescriptionField.text,
                        banner: installationBannerPicker.imageSource,
                        poster: installationPosterPicker.imageSource,
                        icon: installationIconPicker.imageSource,
                        logo: installationLogoPicker.imageSource,
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
                                tooltip: "The game this is an installation of"
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

                                tooltip: "The type of installation. This determines how the installation is launched."
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

                                tooltip: "A title for this particular installation. e.g. \"Definitive Edition\" or \"Modded\""
                            }

                            NblTextBox {
                                id: installationDescriptionField
                                label: "Installation Description"
                                placeholderText: "Optional"
                                width: parent.width

                                onTextEdited: {
                                    installationSettingsTab.modified = true
                                }

                                tooltip: "A description for this particular installation."
                            }

                            NblTagList {
                                id: installationTags
                                label: "Installation Tags"
                                width: parent.width

                                onModified: {
                                    installationSettingsTab.modified = true
                                }

                                tooltip: "Tags that apply to this installation, but not necessarily all installations of its game"
                            }

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
                                    tooltip: "The installation's poster"
                                }
                                NblImagePicker {
                                    id: installationIconPicker
                                    label: "Icon"
                                    implicitHeight: parent.height
                                    Layout.fillWidth: true
                                    onImageSourceChanged: {
                                        installationSettingsTab.modified = true
                                    }
                                    tooltip: "The installation's icon"
                                }
                                NblImagePicker {
                                    id: installationLogoPicker
                                    label: "Logo"
                                    implicitHeight: parent.height
                                    Layout.fillWidth: true
                                    fillMode: Image.PreserveAspectFit
                                    tooltip: "The installation's logo"
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
                                tooltip: "The installation's banner"
                            }

                            NblComboBox {
                                id: installationLogoPositionBox
                                label: "Logo Position"
                                width: parent.width
                                model: [ "Left", "Center", "Right", "Hidden", "Inherit" ]
                                onActivated: {
                                    installationSettingsTab.modified = true
                                }
                                tooltip: "The position the logo will appear over the banner on the installation's detail page"
                            }
                        }
                    }
                }
            }
        }
    }
}
