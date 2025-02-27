import CorPlayer

import Qt.labs.platform
import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: mainWindow

    readonly property alias fileDialog: fileDialog

    function handleDrop(drop) {
        if (drop.hasUrls) {
            if (!CorPlayer.openFiles(drop.urls)) {
                console.error("Failed to open files");
            } else {
                console.log("Opened files");
            }
        }
    }

    Material.theme: Material.System
    Material.accent: "#0582CA"

    height: settings.height
    width: settings.width

    minimumHeight: 540
    minimumWidth: 960

    title: "CorPlayer"
    visible: true

    Component.onCompleted: {
        CorPlayer.initialize();
        CorPlayer.trackManager.persistentState = settings.trackState;
        if (settings.maximized)
            mainWindow.showMaximized();

        controls.volumeControl.volume = settings.volume;
        controls.volumeControl.muted = settings.muted;
        CorPlayer.mediaPlayer.isMuted = Qt.binding(() => {
            return controls.volumeControl.muted;
        });
        CorPlayer.mediaPlayer.volume = Qt.binding(() => {
            return controls.volumeControl.volume;
        });
    }

    onClosing: {
        if (mainWindow.visibility === Window.Maximized) {
            settings.maximized = true;
        } else {
            settings.maximized = false;
            settings.height = mainWindow.height;
            settings.width = mainWindow.width;
        }
    }
    Component.onDestruction: {
        settings.trackState = CorPlayer.trackManager.persistentState;
    }

    Settings {
        id: settings

        property int volume
        property bool muted
        property var trackState
        property bool maximized
        property alias height: mainWindow.height
        property alias width: mainWindow.width
    }

    Connections {
        function onVolumeChanged() {
            if (controls !== null)
                controls.volumeControl.volume = CorPlayer.mediaPlayer.volume;

        }

        function onMutedChanged() {
            if (controls !== null)
                controls.volumeControl.muted = CorPlayer.mediaPlayer.isMuted;

        }

        target: CorPlayer.mediaPlayer
    }

    DropArea {
        anchors.fill: parent
        onDropped: (drop) => {
            return handleDrop(drop);
        }
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
        color: "black"

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // Left sidebar
            Rectangle {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                color: "black"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    Row {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        spacing: 8

                        IconButton {
                            icon.name: "qrc:/icons/fa_plus"
                            tooltipText: "Create New Playlist"
                            onClicked: {
                                console.log("New Playlist");
                            }
                        }

                        IconButton {
                            icon.name: "qrc:/icons/fa_folder"
                            tooltipText: "Load Playlist"
                            onClicked: fileDialog.loadPlaylist();
                        }
                    }

                    // Divider
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        Layout.margins: 8
                        color: "gray"
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        // TODO: add model
                        delegate: PlaylistTrackItem {
                            width: parent.width
                            height: 40
                            name: "Track Name"
                            // selected: false
                            onClicked: {
                                console.log("Switch to playlist")
                            }
                        }
                    }
                }
            }

            // Main content
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#121212"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    // Current playlist header
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Rectangle {
                            Layout.preferredWidth: 192
                            Layout.preferredHeight: 192
                            color: "#282828"

                            Image {
                                anchors.fill: parent
                                anchors.margins: 16
                                // source: "qrc:/icons/fa_music"
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: "Current Playlist"
                                color: "white"
                                font.pixelSize: 48
                                font.weight: Font.Bold
                            }

                            Text {
                                text: tracks.count + " tracks"
                                color: "#b3b3b3"
                                font.pixelSize: 14
                            }

                            Row {
                                spacing: 8

                                // TODO: track playing like other play button
                                Button {
                                    // playing: CorPlayer.playerManager.isPlaying
                                    enabled: CorPlayer.playerManager.canPlay
                                    onClicked: {
                                        if (playing) {
                                            CorPlayer.trackManager.pause()
                                        } else {
                                            CorPlayer.trackManager.play()
                                        }
                                    }
                                }

                                Button {
                                    // icon.name: "qrc:/icons/fa_save"
                                    onClicked: fileDialog.savePlaylist()
                                }
                            }
                        }
                    }

                    // Tracks list header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: "transparent"

                        RowLayout {
                            anchors.fill: parent
                            spacing: 8

                            Text {
                                Layout.preferredWidth: 40
                                text: "#"
                                color: "#b3b3b3"
                                font.pixelSize: 12
                            }

                            Text {
                                Layout.fillWidth: true
                                text: "Title"
                                color: "#b3b3b3"
                                font.pixelSize: 12
                            }

                            Text {
                                Layout.preferredWidth: 200
                                text: "Artist"
                                color: "#b3b3b3"
                                font.pixelSize: 12
                            }

                            Text {
                                Layout.preferredWidth: 200
                                text: "Album"
                                color: "#b3b3b3"
                                font.pixelSize: 12
                            }

                            Text {
                                Layout.preferredWidth: 64
                                text: "Duration"
                                color: "#b3b3b3"
                                font.pixelSize: 12
                            }
                        }
                    }

                    // Tracks list
                    ListView {
                        id: tracks
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: CorPlayer.playlistModel
                        // delegate: TrackListItem {
                        //     width: parent.width
                        //     height: 56
                        //     index: model.index + 1
                        //     title: model.title
                        //     artist: model.artist
                        //     album: model.album
                        //     duration: model.duration
                        //     isPlaying: model.isPlaying
                        //     isCurrentTrack: model.current
                        //     onClicked: {
                        //         console.log("Play track")
                        //     }
                        // }

                        ScrollBar.vertical: ScrollBar {}
                    }
                }
            }
        }

        Rectangle {
            id: playerControls
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 90
            color: "#181818"

            PlaybackControlBar {
                id: controls
                anchors.fill: parent

                focus: true

                Component.onDestruction: {
                    settings.volume = controls.volumeControl.volume;
                    settings.muted = controls.volumeControl.muted;
                }
            }
        }

        // Loader {
        //     id: controlsLoader
        //
        //     Layout.preferredWidth: parent.width
        //     Layout.preferredHeight: parent.height * 0.1
        //     visible: active
        //
        //     sourceComponent: Playbackcontrols {
        //         id: controls
        //
        //         focus: true
        //
        //         Component.onDestruction: {
        //             settings.volume = controls.volumeControl.volume;
        //             settings.muted = controls.volumeControl.muted;
        //         }
        //     }
        // }
    }
}
