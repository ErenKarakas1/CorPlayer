import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: playbackControlBar

    property string title
    property string artist
    property string albumArtist
    property string album
    property string image
    property int albumID

    property int handlePosition: implicitHeight

    signal openArtist();
    signal openAlbum();
    signal openNowPlaying();

    PlaybackControl {
        id: playbackControlItem

        focus: true
        z: 1

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}
