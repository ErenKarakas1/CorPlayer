import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: playbackControlBar

    anchors.fill: parent

    PlaybackControl {
        id: playbackControlItem

        focus: true
        z: 1

        anchors.fill: parent
    }
}
