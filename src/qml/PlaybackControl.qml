import CorPlayer

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: playbackControl

    readonly property int duration: CorPlayer.trackManager.duration
    readonly property bool isPlaying: CorPlayer.playerManager.isPlaying
    readonly property int repeatMode: CorPlayer.trackPlaylistProxyModel.repeatMode
    readonly property int shuffleMode: CorPlayer.trackPlaylistProxyModel.shuffleMode
    readonly property bool playEnabled: CorPlayer.playerManager.canPlay
    readonly property int position: CorPlayer.trackManager.position
    readonly property bool seekable: CorPlayer.mediaPlayer.isSeekable
    readonly property bool skipForwardEnabled: CorPlayer.playerManager.canSkipForward
    readonly property bool skipBackwardEnabled: CorPlayer.playerManager.canSkipBackward

    property bool muted

    signal pause()
    signal play()
    signal playNext()
    signal playPrevious()
    signal seek(int position)

    signal changeRepeatMode()
    signal changeShuffleMode()

    onPause: CorPlayer.trackManager.playPause()
    onPlay: CorPlayer.trackManager.playPause()
    onPlayNext: CorPlayer.trackPlaylistProxyModel.skipNextTrack(PlayerUtils.Manual)
    onPlayPrevious: CorPlayer.trackPlaylistProxyModel.skipPreviousTrack(CorPlayer.mediaPlayer.position)
    onSeek: (position) => {
        return CorPlayer.trackManager.seek(position);
    }
    onChangeRepeatMode: CorPlayer.trackPlaylistProxyModel.setRepeatMode((repeatMode + 1) % 3)
    onChangeShuffleMode: CorPlayer.trackPlaylistProxyModel.setShuffleMode((shuffleMode + 1) % 2)

    Column {
        id: playbackControlLayout

        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        DurationSlider {
            id: durationSlider

            implicitWidth: parent.width * 0.8
            anchors.horizontalCenter: parent.horizontalCenter

            duration: playbackControl.duration
            playEnabled: playbackControl.playEnabled
            position: playbackControl.position
            seekable: playbackControl.seekable

            onSeek: (position) => {
                return playbackControl.seek(position);
            }
        }

        RowLayout {
            id: playbackControlRow

            anchors.horizontalCenter: parent.horizontalCenter

            IconButton {
                id: shuffleButton

                enabled: playbackControl.playEnabled
                icon.name: playbackControl.shuffleMode === 0 ? "qrc:/icons/fa_no_shuffle" : "qrc:/icons/fa_shuffle"
                tooltipText: "Shuffle"
                onClicked: playbackControl.changeShuffleMode()
            }

            IconButton {
                id: goBackwardButton

                enabled: playbackControl.skipBackwardEnabled
                icon.name: "qrc:/icons/fa_backward"
                tooltipText: "Previous"
                onClicked: playbackControl.playPrevious()
            }

            // TODO: skip next changes icon momentarily
            IconButtonWithBackground {
                id: playPauseButton

                Layout.alignment: Qt.AlignHCenter

                enabled: playbackControl.playEnabled
                icon.name: playbackControl.isPlaying ? "qrc:/icons/fa_pause" : "qrc:/icons/fa_play"
                tooltipText: playbackControl.isPlaying ? "Pause" : "Play"
                onClicked: playbackControl.isPlaying ? playbackControl.pause() : playbackControl.play()
            }

            IconButton {
                id: goForwardButton

                enabled: playbackControl.skipForwardEnabled
                icon.name: "qrc:/icons/fa_forward"
                tooltipText: "Next"
                onClicked: playbackControl.playNext()
            }

            IconButton {
                id: repeatButton

                enabled: playbackControl.playEnabled
                icon.name: {
                    const map = {
                        0: "qrc:/icons/fa_no_repeat",
                        1: "qrc:/icons/fa_track_repeat",
                        2: "qrc:/icons/fa_playlist_repeat"
                    };
                    return map[playbackControl.repeatMode];
                }
                tooltipText: {
                    const map = {
                        0: "Repeat Track",
                        1: "Repeat Playlist",
                        2: "Disable Repeat"
                    };
                    return map[playbackControl.repeatMode];
                }
                onClicked: playbackControl.changeRepeatMode()
            }
        }
    }
}
