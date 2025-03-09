pragma ComponentBehavior: Bound

import CorPlayer

import Qt.labs.platform
import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import "dialogs"

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
            CorPlayer.playlistProxyModel.loadPlaylistFromFile(fileDialog.file);
        }
    }

    NewPlaylistDialog {
        id: newPlaylistDialog
        parent: mainWindow.contentItem
        anchors.centerIn: parent
    }

    RenamePlaylistDialog {
        id: renamePlaylistDialog
        parent: mainWindow.contentItem
        anchors.centerIn: parent
    }

    DeletePlaylistDialog {
        id: deletePlaylistDialog
        parent: mainWindow.contentItem
        anchors.centerIn: parent
        playlists: playlists
    }

    Rectangle {
        id: mainView

        anchors.fill: parent
        color: "black"

        RowLayout {
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: playerControls.top
            }
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
                            onClicked: newPlaylistDialog.open();
                        }

                        IconButton {
                            icon.name: "qrc:/icons/fa_folder"
                            tooltipText: "Import Playlist"
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

                    // All tracks section
                    ItemDelegate {
                        Layout.fillWidth: true
                        height: 40

                        contentItem: RowLayout {
                            spacing: 12

                            // Image {
                            //     source: "qrc:/icons/fa_music"
                            //     Layout.preferredWidth: 20
                            //     Layout.preferredHeight: 20
                            // }

                            Text {
                                text: "All Tracks"
                                color: sidebarStack.currentIndex === 0 ? "white" : "#b3b3b3"
                                font.pixelSize: 14
                            }
                        }

                        onClicked: sidebarStack.currentIndex = 0
                    }

                    // Divider
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        Layout.margins: 8
                        color: "gray"
                    }

                    ListView {
                        id: playlists
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: CorPlayer.playlistCollectionModel

                        delegate: ItemDelegate {
                            id: playlistDelegate
                            width: parent.width
                            height: 48

                            required property int index
                            required property int playlistId
                            required property string name
                            required property int trackCount

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8

                                Text {
                                    Layout.fillWidth: true
                                    text: playlistDelegate.name
                                    color: playlistDelegate.ListView.isCurrentItem ? "white" : "#b3b3b3"
                                    font.pixelSize: 14
                                    elide: Text.ElideRight
                                }

                                // Edit button here

                                // Trash button here
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton

                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.LeftButton) {
                                        playlists.currentIndex = playlistDelegate.index
                                        sidebarStack.currentIndex = 1
                                        CorPlayer.playlistProxyModel.loadPlaylistFromDatabase(playlistDelegate.playlistId)
                                    }

                                    if (mouse.button === Qt.RightButton) {
                                        playlistContextMenu.popup()
                                    }
                                }
                            }
                        }
                    }
                }

                Menu {
                    id: playlistContextMenu

                    MenuItem {
                        text: "Rename"
                        onTriggered: {
                            renamePlaylistDialog.playlistIndex = playlists.currentIndex
                            renamePlaylistDialog.playlistName = playlists.currentItem.name
                            renamePlaylistDialog.open()
                        }
                    }

                    MenuItem {
                        text: "Delete"
                        onTriggered: {
                            deletePlaylistDialog.open()
                        }
                    }

                    MenuSeparator {
                    }

                    MenuItem {
                        text: "Export"
                        onTriggered: {
                            fileDialog.savePlaylist()
                        }
                    }
                }
            }

            // Main content
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#121212"

                StackLayout {
                    id: sidebarStack
                    anchors.fill: parent
                    currentIndex: 0

                    // All Tracks view
                    TrackBrowser {
                        id: libraryView
                        model: CorPlayer.trackCollectionModel
                        showHeader: true

                        headerTitle: "Library"
                        headerText: model.rowCount() + " tracks"

                        onTrackDoubleClicked: (index) => {
                            const trackId = model.getTrackId(index)
                            CorPlayer.playTrack(trackId)
                        }
                    }

                    // Current Playlist view
                    ColumnLayout {
                        spacing: 16

                        // Playlist header
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 232
                            color: "#282828"

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 20

                                // Playlist cover
                                Rectangle {
                                    Layout.preferredWidth: 192
                                    Layout.preferredHeight: 192
                                    color: "#181818"

                                    // Image {
                                    //     anchors.fill: parent
                                    //     anchors.margins: 48
                                    //     source: "qrc:/icons/fa_list"
                                    //     sourceSize: Qt.size(96, 96)
                                    // }
                                }

                                // Playlist info
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Text {
                                        text: "PLAYLIST"
                                        color: "#b3b3b3"
                                        font.pixelSize: 12
                                    }

                                    Text {
                                        text: playlists.currentIndex >= 0 ? playlists.currentItem.name : "No playlist selected"
                                        color: "white"
                                        font.pixelSize: 48
                                        font.weight: Font.Bold
                                    }

                                    Text {
                                        text: playlists.currentIndex >= 0 ? playlists.currentItem.trackCount + " tracks" : ""
                                        color: "#b3b3b3"
                                        font.pixelSize: 14
                                    }

                                    // Playlist actions
                                    RowLayout {
                                        spacing: 16

                                        IconButton {
                                            icon.name: CorPlayer.playerManager.isPlaying
                                                ? "qrc:/icons/fa_pause"
                                                : "qrc:/icons/fa_play"
                                            enabled: playlists.currentIndex >= 0
                                            onClicked: {
                                                if (CorPlayer.playerManager.isPlaying) {
                                                    playerControls.pause()
                                                } else {
                                                    playerControls.play()
                                                }
                                            }
                                        }

                                        IconButton {
                                            icon.name: "qrc:/icons/fa_plus"
                                            tooltipText: "Add tracks"
                                            enabled: playlists.currentIndex >= 0
                                            onClicked: addTracksDialog.open()
                                        }

                                        IconButton {
                                            icon.name: "qrc:/icons/fa_ellipsis"
                                            tooltipText: "More options"
                                            enabled: playlists.currentIndex >= 0
                                            onClicked: playlistOptionsMenu.popup()
                                        }
                                    }
                                }
                            }
                        }

                        // Playlist tracks
                        TrackBrowser {
                            id: playlistView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: CorPlayer.playlistModel
                            showHeader: false
                            dragEnabled: true

                            onTrackDoubleClicked: (index) => {
                                CorPlayer.playlistProxyModel.switchTo(index)
                                playerControls.play()
                            }
                        }
                    }
                }

                // Playlist options menu
                Menu {
                    id: playlistOptionsMenu

                    MenuItem {
                        text: "Sort by"
                        enabled: false  // TODO: Implement sorting
                    }

                    MenuSeparator {
                    }

                    MenuItem {
                        text: "Export playlist"
                        onTriggered: fileDialog.savePlaylist()
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
