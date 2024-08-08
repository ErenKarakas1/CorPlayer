import QtQuick 2.6
import QtQuick.Controls 2.0
import QtMultimedia 6.0
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 600
    height: 800
    visible: true

    FileDialog {
        id: fileDialog
        title: "Select a Music File"
        nameFilters: ["Audio files (*.mp3 *.wav *.ogg *.opus *.flac)"]
        onAccepted: {
            mediaPlayerBackend.source = fileDialog.currentFile
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Image {
            source: mediaPlayerBackend.coverArtUrl
            width: 200
            height: 200
            fillMode: Image.PreserveAspectFit
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Title: " + mediaPlayerBackend.title
            font.pointSize: 24
            color: "red"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Artist: " + mediaPlayerBackend.artist
            font.pointSize: 20
            color: "gray"
            Layout.alignment: Qt.AlignHCenter
        }

        Button {
            id: playPauseButton
            text: mediaPlayerBackend.playbackState === MediaPlayer.PlayingState ? "Pause" : "Play"
            Layout.alignment: Qt.AlignHCenter
            onClicked: { mediaPlayerBackend.playPause() }
        }

        Button {
            text: "Open File Dialog"
            Layout.alignment: Qt.AlignHCenter
            onClicked: { fileDialog.open() }
        }
    }
}
