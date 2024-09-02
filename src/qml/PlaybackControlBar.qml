import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: playbackControlBar

    anchors.fill: parent
    anchors.topMargin: 5
    anchors.bottomMargin: 5

    PlaybackControl {
        id: playbackControlItem

        focus: true
        anchors.fill: parent
    }
}
