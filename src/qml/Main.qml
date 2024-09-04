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

    readonly property alias fileDialog: fileDialog
    readonly property alias controlBar: controlBarLoader.item

    function handleDrop(drop) {
        console.log("test");
        if (drop.hasUrls) {
            if (!CorPlayer.openFiles(drop.urls)) {
                console.error("Failed to open files");
            } else {
                console.log("Opened files");
            }
        }
    }

    Material.theme: Material.System
    height: 720
    width: 1280
    minimumHeight: 540
    minimumWidth: 960

    // TODO: Decide on a minimum size for the window

    title: "CorPlayer"
    visible: true

    Connections {
        target: CorPlayer.mediaPlayer
        function onVolumeChanged() {
            if (controlBar !== null) {
                controlBar.volumeControl.volume = CorPlayer.mediaPlayer.volume
            }
        }

        function onMutedChanged() {
            if (controlBar !== null) {
                controlBar.volumeControl.muted = CorPlayer.mediaPlayer.isMuted
            }
        }
    }

    Component.onCompleted: {
        CorPlayer.initialize();
        CorPlayer.mediaPlayer.isMuted = Qt.binding(() => controlBar.volumeControl.muted);
        CorPlayer.mediaPlayer.volume = Qt.binding(() => controlBar.volumeControl.volume);
    }

    DropArea {
        anchors.fill: parent

        onDropped: drop => handleDrop(drop)
    }
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

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height * 0.9
                color: "black"

                RowLayout {
                    Layout.preferredWidth: parent.width
                    Layout.alignment: Qt.AlignHCenter

                    Button {
                        Layout.alignment: Qt.AlignLeft
                        text: "Open Playlist"

                        onClicked: fileDialog.loadPlaylist()
                    }

                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: "Save Playlist"

                        onClicked: fileDialog.savePlaylist()
                    }
                }
            }
            Loader {
                id: controlBarLoader

                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: active

                sourceComponent: PlaybackControlBar {
                    id: controlBar

                    focus: true
                }
            }
        }
    }
}
