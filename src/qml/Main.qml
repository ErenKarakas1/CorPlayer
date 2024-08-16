import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtMultimedia
import QtQuick.Layouts

import Qt.labs.platform

ApplicationWindow {
    id: root
    title: qsTr("CorPlayer")
    width: 1280
    height: 720
    visible: true

    Material.theme: Material.System

    // TODO: Decide on a minimum size for the window
    minimumWidth: 960
    minimumHeight: 540

    FileDialog {
        id: fileDialog
        title: "Select a Music File"
        nameFilters: ["Audio files (*.mp3 *.wav *.ogg *.opus *.flac)"]
        onAccepted: {
            mediaPlayerBackend.source = fileDialog.currentFile
        }
    }

    StackLayout {
        anchors.fill: parent

        Button {
            text: "Select file"
            onClicked: { fileDialog.open() }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter

            PlaybackControl {
                id: playbackControl
                focus: true
                visible: true
            }
        }
    }
}

