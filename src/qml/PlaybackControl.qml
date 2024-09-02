import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CorPlayer

import "components"

FocusScope {
    id: playbackControl

    readonly property int duration: CorPlayer.trackManager.trackDuration
    readonly property bool isPlaying: CorPlayer.playerManager.musicPlaying
    property bool muted
    readonly property bool playEnabled: CorPlayer.playerManager.playControlEnabled
    readonly property int position: CorPlayer.trackManager.trackPosition
    readonly property bool seekable: CorPlayer.mediaPlayer.isSeekable

    signal pause
    signal play
    signal playNext
    signal playPrevious
    signal seek(int position)

    onPause: CorPlayer.trackManager.playPause()
    onPlay: CorPlayer.trackManager.playPause()
    onPlayNext: CorPlayer.trackPlaylistProxyModel.skipNextTrack(PlayerUtils.Manual)
    onPlayPrevious: CorPlayer.trackPlaylistProxyModel.skipPreviousTrack(CorPlayer.mediaPlayer.position)
    onSeek: position => CorPlayer.trackManager.trackSeek(position)

    ColumnLayout {
        id: playbackControlLayout
        anchors.fill: parent
        spacing: 10

        DurationSlider {
            id: durationSlider

            Layout.maximumWidth: parent.width * 0.5
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

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

            ToolButton {
                id: goBackwardButton

                enabled: playbackControl.playEnabled
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

                enabled: playbackControl.playEnabled
                icon.name: "qrc:/icons/fa_forward"
                onClicked: playbackControl.playNext()
            }
        }
    }
}
