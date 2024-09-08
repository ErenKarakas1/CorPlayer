import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CorPlayer

import "components"

FocusScope {
    id: playbackControl

    readonly property int duration: CorPlayer.trackManager.trackDuration
    readonly property bool isPlaying: CorPlayer.playerManager.musicPlaying
    readonly property int repeatMode: CorPlayer.trackPlaylistProxyModel.repeatMode
    readonly property int shuffleMode: CorPlayer.trackPlaylistProxyModel.shuffleMode
    readonly property bool playEnabled: CorPlayer.playerManager.playControlEnabled
    readonly property int position: CorPlayer.trackManager.trackPosition
    readonly property bool seekable: CorPlayer.mediaPlayer.isSeekable
    readonly property bool skipForwardEnabled: CorPlayer.playerManager.skipForwardControlEnabled
    readonly property bool skipBackwardEnabled: CorPlayer.playerManager.skipBackwardControlEnabled

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
    onSeek: position => CorPlayer.trackManager.trackSeek(position)

    onChangeRepeatMode: CorPlayer.trackPlaylistProxyModel.setRepeatMode((repeatMode + 1) % 3)
    onChangeShuffleMode: CorPlayer.trackPlaylistProxyModel.setShuffleMode((shuffleMode + 1) % 2)

    ColumnLayout {
        id: playbackControlLayout
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter

        DurationSlider {
            id: durationSlider

            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.preferredHeight: 1
            Layout.maximumWidth: parent.width * 0.7

            duration: playbackControl.duration
            playEnabled: playbackControl.playEnabled
            position: playbackControl.position
            seekable: playbackControl.seekable

            onSeek: position => playbackControl.seek(position)
        }

        RowLayout {
            id: playbackControlRow

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.preferredHeight: 4

            ToolButton {
                id: shuffleButton

                enabled: playbackControl.playEnabled
                icon.name: playbackControl.shuffleMode === 0 ? "qrc:/icons/fa_no_shuffle" : "qrc:/icons/fa_shuffle"
                onClicked: playbackControl.changeShuffleMode();
            }

            ToolButton {
                id: goBackwardButton

                enabled: skipBackwardEnabled
                icon.name: "qrc:/icons/fa_backward"
                onClicked: playbackControl.playPrevious()
            }

            MaterialButton {
                id: playPauseButton

                Layout.alignment: Qt.AlignHCenter

                enabled: playbackControl.playEnabled
                icon.name: playbackControl.isPlaying ? "qrc:/icons/fa_pause" : "qrc:/icons/fa_play"
                onClicked: playbackControl.isPlaying ? playbackControl.pause() : playbackControl.play()
            }

            ToolButton {
                id: goForwardButton

                enabled: skipForwardEnabled
                icon.name: "qrc:/icons/fa_forward"
                onClicked: playbackControl.playNext()
            }

            ToolButton {
                id: repeatButton

                enabled: playbackControl.playEnabled
                icon.name: {
                    const map = {
                        0: "qrc:/icons/fa_no_repeat",
                        1: "qrc:/icons/fa_track_repeat",
                        2: "qrc:/icons/fa_playlist_repeat"
                    }
                    return map[playbackControl.repeatMode]
                }
                onClicked: playbackControl.changeRepeatMode()
            }
        }
    }
}
