import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CorPlayer

RowLayout {
    id: root
    anchors.fill: parent
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
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        Text {
            id: trackTitle
            text: "Track Title"
            font.bold: true
        }

        Text {
            id: trackArtist
            text: "Track Artist"
        }
    }

}