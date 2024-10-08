import CorPlayer

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: trackInfo

    property string title: CorPlayer.metadataManager.title
    property string artist: CorPlayer.metadataManager.artist

    RowLayout {
        spacing: 10

        //wip
        Rectangle {
            id: albumArt

            width: 50
            height: 50
            color: "red"
        }

        //wip
        ColumnLayout {
            id: trackInfoLayout

            spacing: 5
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            Text {
                id: trackTitle

                text: title
                font.bold: true
                color: "white"
            }

            Text {
                id: trackArtist

                text: artist
                color: "white"
            }
        }
    }
}
