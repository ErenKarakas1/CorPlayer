import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property alias muted: muteButton.checked
    property real volume: slider.value

    width: trackControl.width

    RowLayout {
        id: trackControl
        spacing: 10

        RoundButton {
            id: muteButton
            text: "Mute"
            checkable: true
        }

        Slider {
            id: slider
            width: 136

            enabled: !root.muted
            value: 1
        }
    }
}
