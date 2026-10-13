pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Nebula.Controls

Item {

    id: root

    property string label: ""
    required property string accountProvider
    property bool showDefaultOption: false

    readonly property AccountModel model: AccountModel {
        providerId: root.accountProvider
    }

    property int selectedId
    property string selectedName: showDefaultOption ? "Default" : "No Registered Accounts"
    property string selectedIcon: ""
    property bool selectedSmooth: false

    implicitHeight: 48 + 30

    onSelectedIdChanged: {

        if(selectedId == -1) {
            if(!showDefaultOption && model.rowCount() > 0) {
                selectedId = model.data(model.index(0,0), 0)
            }
            return
        }

        for(let i = 0 ; i < model.rowCount() ; i++) {
            let index = model.index(i, 0)
            let id = model.data(index, 0)
            if(id == selectedId) {
                selectedName = model.data(index, 1)
                selectedIcon = model.data(index, 2)
                selectedSmooth = !model.data(index, 3)
                return
            }
        }

        selectedId = -1
    }

    Component.onCompleted: {
        if(!root.showDefaultOption && model.rowCount() > 0) {
            selectedId = model.data(model.index(0,0), 0)
        } else {
            selectedId = -1
        }
    }

    Connections {
        target: root.model
        function onRowsInserted(parent, first, last) {

            if(root.selectedId == -1) {
                let index = root.model.index(first, 0, parent)
                let id = root.model.data(index, 0)
                root.selectedId = id
                root.selectedName = root.model.data(index, 1)
                root.selectedIcon = root.model.data(index, 2)
                root.selectedSmooth = !root.model.data(index, 3)
                return;
            }

            for(let i = first ; i < last ; i++) {
                let index = root.model.index(i, 0, parent)
                let id = root.model.data(index, 0)
                if(id == root.selectedId) {
                    root.selectedName = root.model.data(index, 1)
                    root.selectedIcon = root.model.data(index, 2)
                    root.selectedSmooth = !root.model.data(index, 3)
                    return
                }
            }
        }
        function onRowsAboutToBeRemoved(parent, first, last) {
            for(let i = first ; i < last ; i++) {
                let index = root.model.index(i, 0, parent)
                let id = root.model.data(index, 0)
                if(id == root.selectedId) {
                    root.selectedId = -1;
                    root.selectedName = root.showDefaultOption ? "Default" : "None"
                    root.selectedIcon = ""
                    return
                }
            }
        }
    }


    Column {

        width: parent.width

        Item {

            width: parent.width
            height: 30

            Rectangle {
                width: parent.width
                height: 2
                y: (parent.height / 2) - 1
                color: Theme.colors.border
            }

            Rectangle {
                id: labelBg
                x: 12
                visible: root.label != null && root.label.length > 0
                width: labelLabel.width + 12
                height: parent.height
                color: Theme.colors.layer0
            }

            Label {
                id: labelLabel
                anchors.centerIn: labelBg
                font.family: Theme.font
                font.pixelSize: 16
                font.weight: 500
                text: root.label
                color: Theme.colors.textDim
            }
        }

        Item {

            width: parent.width
            height: 48

            Rectangle {
                anchors.fill: parent
                color: mouseArea.containsMouse ? Theme.colors.layer1 : "transparent"
                radius: 4
            }

            RowLayout {
                anchors.fill: parent

                NblInlineIcon {
                    Layout.fillHeight: true
                    Layout.preferredWidth: height
                    Layout.margins: 8
                    imageRadius: 8;
                    icon: root.selectedIcon
                    smoothImage: root.selectedSmooth
                    color: Theme.colors.text
                    colorize: false
                }

                Label {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    font.pixelSize: 16
                    font.weight: 500
                    text: root.selectedName
                    color: Theme.colors.text
                    verticalAlignment: Qt.AlignVCenter
                }

                Label {
                    Layout.fillHeight: true
                    Layout.preferredWidth: height
                    font.pixelSize: 16
                    font.weight: 600
                    text: ""
                    color: Theme.colors.text
                    verticalAlignment: Qt.AlignVCenter
                    horizontalAlignment: Qt.AlignHCenter
                }
            }
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    if(popup.rowCount > 0) {
                        popup.open()
                    }
                }
            }
        }
    }

    Popup {

        id: popup

        y: root.height

        property int rowCount: root.model.rowCount() + (root.showDefaultOption ? 1 : 0)

        width: root.width
        height: Math.min(rowCount * 48, 48 * 3) + 16
        padding: 8

        background: Rectangle {
            color: Theme.colors.layer0
            border.width: 2
            border.color: Theme.colors.border
            radius: 8
        }
        contentItem: NblScrollView {
            Column {

                width: parent.width

                Item {

                    visible: root.showDefaultOption

                    width: parent.width
                    height: 48

                    Rectangle {
                        anchors.fill: parent
                        color: defaultMouseArea.containsMouse ? Theme.colors.layer1 : "transparent"
                        radius: 4
                    }

                    RowLayout {

                        anchors.fill: parent

                        NblInlineIcon {
                            Layout.fillHeight: true
                            Layout.preferredWidth: height
                            Layout.margins: 8
                            imageRadius: 8;
                            icon: ""
                            color: Theme.colors.text
                        }

                        Label {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            font.pixelSize: 16
                            font.weight: 500
                            text: "Default"
                            color: Theme.colors.text
                            verticalAlignment: Qt.AlignVCenter
                        }
                    }

                    MouseArea {
                        id: defaultMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("Selected default")
                            root.selectedId = -1
                            root.selectedName = "Default"
                            root.selectedIcon = ""
                            popup.close()
                        }
                    }
                }


                Repeater {

                    model: root.model
                    delegate: Item {

                        id: accountItem

                        width: parent.width
                        height: 48

                        required property var modelData

                        Rectangle {
                            anchors.fill: parent
                            color: accountMouseArea.containsMouse ? Theme.colors.layer1 : "transparent"
                            radius: 4
                        }

                        RowLayout {

                            anchors.fill: parent

                            NblInlineIcon {
                                Layout.fillHeight: true
                                Layout.preferredWidth: height
                                Layout.margins: 8
                                imageRadius: 8;
                                icon: accountItem.modelData.icon
                                smoothImage: !accountItem.modelData.iconNearestScaling
                                color: Theme.colors.text
                                colorize: false
                            }

                            Label {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                font.pixelSize: 16
                                font.weight: 500
                                text: accountItem.modelData.name
                                color: Theme.colors.text
                                verticalAlignment: Qt.AlignVCenter
                            }
                        }

                        MouseArea {
                            id: accountMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.selectedId = accountItem.modelData.id
                                root.selectedName = accountItem.modelData.name
                                root.selectedIcon = accountItem.modelData.icon
                                root.selectedSmooth = !accountItem.modelData.iconNearestScaling
                                popup.close()
                            }
                        }
                    }
                }
            }
        }
    }
}
