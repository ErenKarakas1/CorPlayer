import CorPlayer

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    title: "Delete Playlist"
    modal: true
    standardButtons: Dialog.Yes | Dialog.No
    width: 300

    property var playlists

    ColumnLayout {
        width: parent.width
        spacing: 16

        Text {
            Layout.fillWidth: true
            text: "Are you sure you want to delete this playlist?"
            color: "#b3b3b3"
            wrapMode: Text.Wrap
        }
    }

    onAccepted: {
        if (dialog.playlists.currentIndex >= 0) {
            CorPlayer.playlistCollectionModel.removePlaylist(
                dialog.playlists.currentIndex
            )
        }
    }
}
