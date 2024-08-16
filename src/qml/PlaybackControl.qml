import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"

FocusScope {
    id: root

    RowLayout {
        id: playbackControl
        anchors.fill: parent

        IconRoundButton {
            id: goBackward
            symbol: "\uf048"
        }

        IconRoundButton {
            id: playPause
            symbol: "\uf04b"
        }

        IconRoundButton {
            id: goForward
            symbol: "\uf051"
        }

        TrackControl {
            id: trackControl
        }
    }
}
