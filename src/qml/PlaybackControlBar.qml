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
        anchors {
            rightMargin: 40
            leftMargin: 20
        }

        TrackInfo {
            id: trackInfo

            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
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

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
        }
    }
}
