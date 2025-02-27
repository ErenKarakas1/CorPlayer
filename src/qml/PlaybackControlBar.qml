import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Layouts

import "components"

FocusScope {
    id: playbackControlBar

    anchors.fill: parent

    property alias volumeControl: volumeControl

    RowLayout {
        id: playbackControlBarLayout

        anchors.fill: parent
        anchors.margins: 20

        Loader {
            id: trackInfoLoader

            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1

            sourceComponent: TrackInfo {
                id: trackInfo

                title: CorPlayer.trackManager?.trackMetadata?.title ?? ""
                artist: CorPlayer.trackManager?.trackMetadata?.artist ?? ""
                image: CorPlayer.trackManager?.trackMetadata?.coverUrl ?? ""
            }
        }

        PlaybackControl {
            id: playbackControlItem

            focus: true

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 6
        }

        VolumeControl {
            id: volumeControl

            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
        }
    }
}
