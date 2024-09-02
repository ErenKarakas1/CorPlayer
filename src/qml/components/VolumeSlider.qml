import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import CorPlayer
import "../.."

GridLayout {
    id: root

    property int volume
    property color labelColor
    property bool muted
    property int position

    signal changeVolume(int position)

    columns: 3
    rowSpacing: 0
    rows: 2

    onPositionChanged: {
        if (!slider.pressed) {
            slider.value = position;
        }
    }

    Connections {
        /*function onSetMuted() {
            slider.value = 0;
        }

        target: CorPlayer.trackPlaylistProxyModel*/
    }
    /*TextMetrics {
        id: volumeTextMetrics
        text: "0"
    }*/

    Label {
        id: positionLabel

        Layout.alignment: Qt.AlignVCenter
        Layout.fillHeight: true
        color: root.labelColor
        verticalAlignment: Text.AlignVCenter

    }
    StepSlider {
        id: slider

        Layout.alignment: Qt.AlignVCenter
        Layout.column: 1
        Layout.columnSpan: 3
        Layout.fillHeight: true

        from: 0
        keyStepSize: 1
        //live: true
        to: 100
        wheelStepSize: 1

        onMoved: root.changeVolume(value)


    }
}
