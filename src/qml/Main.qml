import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtMultimedia
import CorPlayer

import Qt.labs.platform

ApplicationWindow {
    id: mainWindow

    Material.theme: Material.System
    height: 720
    minimumHeight: 540

    // TODO: Decide on a minimum size for the window
    minimumWidth: 960
    title: qsTr("CorPlayer")
    visible: true
    width: 1280

    Connections {
        target: CorPlayer.trackPlaylistProxyModel
        function onPlaylistLoadFailed() {
            console.error("Failed to load playlist");
        }
    }

    function handleDrop(drop) {
        console.log("test")
        if (drop.hasUrls) {
            if (!CorPlayer.openFiles(drop.urls)) {
                console.error("Failed to open files");
            } else {
                console.log("Opened files");
            }
        }
    }

    DropArea {
        anchors.fill: parent
        onDropped: drop => handleDrop(drop)
    }

    readonly property alias fileDialog: fileDialog

    FileDialog {
        id: fileDialog

        function loadPlaylist() {
            fileDialog.nameFilters = ["Playlist File (*.m3u8 *.m3u)"];
            fileDialog.fileMode = FileDialog.OpenFile;
            fileDialog.file = '';
            fileDialog.open();
        }
        function savePlaylist() {
            fileDialog.nameFilters = ["Playlist File (*.m3u8 *.m3u)"];
            fileDialog.defaultSuffix = 'm3u8';
            fileDialog.fileMode = FileDialog.SaveFile;
            fileDialog.file = '';
            fileDialog.open();
        }

        folder: StandardPaths.writableLocation(StandardPaths.MusicLocation)

        onAccepted: {
            CorPlayer.trackPlaylistProxyModel.loadPlaylist(fileDialog.file);
        }
    }

    Rectangle {
        id: mainView
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Loader {
                id: controlBarLoader
                visible: active

                Layout.fillWidth: true

                sourceComponent: PlaybackControlBar {
                    id: controlBar

                    focus: true
                }
            }
        }
    }

    Component.onCompleted: {
        CorPlayer.initialize();
    }
}

