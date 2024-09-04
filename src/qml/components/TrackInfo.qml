import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CorPlayer

Item {
    id: trackInfo

    property string title: CorPlayer.metadataManager.title
    property string artist: CorPlayer.metadataManager.artist

    RowLayout {
        spacing: 10

        Rectangle { //wip
            id: albumArt
            width: 50
            height: 50
            color: "red"
        }

        ColumnLayout { //wip
            id: trackInfoLayout
            spacing: 5
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            Text {
                id: trackTitle
                text: title
                font.bold: true
            }

            Text {
                id: trackArtist
                text: artist
            }
        }
    }
}
