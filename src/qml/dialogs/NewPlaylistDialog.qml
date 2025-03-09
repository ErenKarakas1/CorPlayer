import CorPlayer

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    title: "Create New Playlist"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 300

    ColumnLayout {
        width: parent.width
        spacing: 16

        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: "Playlist name"
            selectByMouse: true
        }
    }

    onAccepted: {
        if (nameField.text.length > 0) {
            CorPlayer.playlistCollectionModel.createPlaylist(nameField.text)
            nameField.text = ""
        }
    }
}
