import QtQuick
import QtQuick.Layouts

Item {
    id: trackInfo

    property string title
    property string artist
    property string image

    onImageChanged: {
        if (image && image !== "") {
            trackImage.source = image.toString().startsWith("image://cover/") ? image : "image://cover/" + image;
        } else {
            trackImage.source = trackImage.fallback;
        }
    }

    RowLayout {
        spacing: 10

        ImageWithFallback {
            id: trackImage

            sourceSize {
                width: 50
                height: 50
            }
            fillMode: Image.PreserveAspectFit
            async: true
            mipmap: true
        }

        //wip
        ColumnLayout {
            id: trackInfoLayout

            spacing: 5
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            Text {
                id: trackTitle

                text: trackInfo.title
                font.bold: true
                color: "white"
            }

            Text {
                id: trackArtist

                text: trackInfo.artist
                color: "white"
            }
        }
    }
}
