import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CorPlayer

import "components"

FocusScope {
    id: playbackControl

    readonly property int position: CorPlayer.trackManager.trackPosition
    readonly property int duration: CorPlayer.trackManager.trackDuration
    readonly property bool isPlaying: CorPlayer.playerManager.musicPlaying
    readonly property bool seekable: CorPlayer.mediaPlayer.isSeekable
    readonly property bool playEnabled: CorPlayer.playerManager.playControlEnabled

    property bool muted

    signal play()
    signal pause()
    signal playNext()
    signal playPrevious()
    signal seek(int position)

    onSeek: position => CorPlayer.trackManager.trackSeek(position)
    onPlay: CorPlayer.trackManager.playPause()
    onPause: CorPlayer.trackManager.playPause()
    onPlayPrevious: CorPlayer.trackPlaylistProxyModel.skipPreviousTrack(CorPlayer.mediaPlayer.position)
    onPlayNext: CorPlayer.trackPlaylistProxyModel.skipNextTrack(PlayerUtils.Manual)

    RowLayout {
        id: playbackControlLayout

        anchors.fill: parent
        spacing: 0

        IconRoundButton {
            id: goBackwardButton

            symbol: "\uf048"
            onClicked: playbackControl.playPrevious()
        }

        IconRoundButton {
            id: playPauseButton

            symbol: "\uf04b"
            onClicked: playbackControl.isPlaying ? playbackControl.pause() : playbackControl.play()
        }

        IconRoundButton {
            id: goForwardButton

            symbol: "\uf051"
            onClicked: playbackControl.playNext()
        }

        DurationSlider {
            Layout.fillWidth: true
            Layout.fillHeight: true
            position: playbackControl.position
            duration: playbackControl.duration
            seekable: playbackControl.seekable
            playEnabled: playbackControl.playEnabled
            onSeek: position => playbackControl.seek(position)
        }
    }
}
