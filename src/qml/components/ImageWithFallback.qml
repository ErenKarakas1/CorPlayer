import QtQuick

Rectangle {
    id: imageWithFallback
    color: "transparent"

    property url source
    property url fallback: "qrc:/icons/default_cover.png"  // TODO: Add a default fallback

    property alias sourceSize: image.sourceSize
    property alias fillMode: image.fillMode
    property alias async: image.asynchronous
    property alias mipmap: image.mipmap

    readonly property alias status: image.status

    implicitWidth: 50
    implicitHeight: 50

    onSourceChanged: {
        image.source = getImageUrl()
    }

    function getImageUrl() {
        if (source === '' || source === undefined) {
            return fallback;
        } else {
            return source;
        }
    }

    Image {
        id: image
        anchors.fill: parent
        cache: false
        source: getImageUrl()

        onStatusChanged: {
            switch(status) {
                case Image.Ready:
                    opacity = 1;
                    break;
                case Image.Loading:
                    opacity = 0;
                    break;
                case Image.Error:
                    source = imageWithFallback.fallback;
                    break;
            }
        }
    }
}
