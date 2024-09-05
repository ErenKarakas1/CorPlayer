import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Layouts

import "components"

FocusScope {
    id: playbackControlBar

    anchors.fill: parent
    anchors.topMargin: 10
    anchors.bottomMargin: 5
    anchors.leftMargin: 15
    anchors.rightMargin: 15

    property alias volumeControl: volumeControl

    RowLayout {
        id: playbackControlBarLayout
        anchors.fill: parent
        spacing: 10

        TrackInfo {
            id: trackInfo

            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width * 0.1
            Layout.maximumWidth: parent.width * 0.1
        }

        PlaybackControl {
            id: playbackControlItem

            focus: true
            height: parent.height
            width: parent.width

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.7

        }

        VolumeControl {
            id: volumeControl

            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: parent.width * 0.1
        }
    }
}
