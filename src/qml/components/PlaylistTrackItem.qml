import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: playlistTrackItem

    property int index
    property string name
    property string artist
    property string album
    property string duration
    property bool isPlaying
    property bool isCurrent

    signal clicked(int index)

    height: 56
    width: parent.width

    color: mouseArea.containsMouse ? "black" : (isCurrent ? "blue" : "transparent")

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Playlist Item index or playing indicator
        Item {
            Layout.preferredWidth: 40
            Layout.fillHeight: true

            Text {
                anchors.centerIn: parent
                text: playlistTrackItem.index + 1
                color: "white"
                font.pixelSize: 16
                visible: !playlistTrackItem.isPlaying
            }

            // TODO: add icon
            Rectangle {
                id: playingIndicator
                anchors.fill: parent
                color: "white"
                visible: playlistTrackItem.isPlaying
            }
        }

        // Track Info
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            Text {
                Layout.fillWidth: true
                text: playlistTrackItem.name
                color: "white"
                font.pixelSize: 16
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: playlistTrackItem.artist
                color: "white"
                font.pixelSize: 14
                elide: Text.ElideRight
            }
        }

        Text {
            Layout.preferredWidth: 200
            text: playlistTrackItem.album
            color: "white"
            font.pixelSize: 14
            elide: Text.ElideRight
        }

        Text {
            Layout.preferredWidth: 64
            text: playlistTrackItem.duration
            color: "white"
            font.pixelSize: 14
            elide: Text.ElideRight
        }

        // More button
        Image {
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            source: "qrc:/icons/fa_more"
            sourceSize: Qt.size(16, 16)
            visible: mouseArea.containsMouse

            MouseArea {
                anchors.fill: parent
                onClicked: itemContextMenu.open()
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: playlistTrackItem.clicked(playlistTrackItem.index)
    }

    Menu {
        id: itemContextMenu

        MenuItem {
            text: "Remove"
            onTriggered: {
                console.log("Remove")
            }
        }

        MenuItem {
            text: "Play next"
            onTriggered: {
                console.log("Play next")
            }
        }

        MenuItem {
            text: "Add to queue"
            onTriggered: {
                console.log("Add to queue")
            }
        }
    }
}
