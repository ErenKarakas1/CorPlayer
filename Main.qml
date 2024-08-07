import QtQuick 2.6
import QtQuick.Controls 2.0
import QtMultimedia 6.0
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 300
    height: 480
    visible: true

    FileDialog {
        id: fileDialog
        title: "Open Media File"
        nameFilters: ["Media Files (*.mp3 *.mp4 *.wav *.avi *.opus *.flac)"]
        onAccepted: {
            console.log("Selected file: " + fileDialog.fileUrl)
            mediaPlayerBackend.source = fileDialog.fileUrl.toString()
        }
    }

    Column {
        spacing: 10
        anchors.centerIn: parent

        TextField {
            id: sourceField
            placeholderText: "Enter media file path"
            text: mediaPlayerBackend.source
            onTextChanged: mediaPlayerBackend.source = text
        }

        Button {
            text: "Select File"
            onClicked: fileDialog.open()
        }

        Button {
            text: mediaPlayerBackend.playing ? "Pause" : "Play"
            onClicked: mediaPlayerBackend.playing = !mediaPlayerBackend.playing
        }

        MediaPlayer {
            id: mediaPlayer
            source: mediaPlayerBackend.source
        }
    }
}
