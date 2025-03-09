import CorPlayer

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dialog
    title: "Rename Playlist"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 300

    property int playlistIndex: -1
    property string playlistName: ""

    ColumnLayout {
        width: parent.width
        spacing: 16

        TextField {
            id: nameField
            Layout.fillWidth: true
            text: dialog.playlistName
            selectByMouse: true
        }
    }

    onAccepted: {
        if (nameField.text.length > 0) {
            CorPlayer.playlistCollectionModel.renamePlaylist(
                playlistIndex,
                nameField.text
            )
        }
    }
}
