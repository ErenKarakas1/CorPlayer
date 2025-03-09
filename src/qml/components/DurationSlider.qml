import CorPlayer

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridLayout {
    id: root

    property int duration
    property color labelColor: "#ffffff"
    property bool labelsInline: true
    property bool playEnabled
    property int position
    property bool seekable

    signal seek(int position)

    rows: 2
    columns: 5
    rowSpacing: 0
    columnSpacing: 10

    onDurationChanged: {
        slider.to = duration / 1000;
    }

    onPositionChanged: {
        if (!slider.pressed) {
            slider.value = position / 1000;
        }
    }

    Connections {
        function onClearPlaylistPlayer() {
            slider.value = 0;
        }

        target: CorPlayer.playlistProxyModel
    }

    TextMetrics {
        id: durationTextMetrics

        text: "00:00:00"
    }

    Label {
        id: positionLabel

        text: timeWatchdog.progressDuration
        color: root.labelColor

        Layout.row: root.labelsInline ? 0 : 1
        Layout.column: root.labelsInline ? 0 : 1
        Layout.alignment: Qt.AlignVCenter
        Layout.fillHeight: true
        Layout.preferredWidth: (durationTextMetrics.boundingRect.width - durationTextMetrics.boundingRect.x)
        Layout.rightMargin: !root.labelsInline ? 0 : 5
        horizontalAlignment: root.labelsInline ? Text.AlignRight : Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        TrackProgressWatchdog {
            id: timeWatchdog

            position: root.position
        }
    }

    StepSlider {
        id: slider

        Layout.row: 0
        Layout.column: 1
        Layout.columnSpan: 3
        Layout.alignment: Qt.AlignVCenter
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.leftMargin: !root.labelsInline ? 0 : 5
        Layout.rightMargin: !root.labelsInline ? 0 : 5

        live: true
        enabled: root.seekable && root.playEnabled

        // from, to and value of Slider are rescaled to seconds to avoid integer overflow issues
        from: 0
        to: root.duration / 1000

        keyStepSize: 5
        wheelStepSize: 10

        onMoved: root.seek(value * 1000)
    }

    Label {
        id: durationLabel

        text: durationWatchdog.progressDuration
        color: root.labelColor

        Layout.row: root.labelsInline ? 0 : 1
        Layout.column: root.labelsInline ? 4 : 3
        Layout.alignment: Qt.AlignVCenter
        Layout.fillHeight: true
        Layout.leftMargin: !root.labelsInline ? 0 : 5
        Layout.preferredWidth: (durationTextMetrics.boundingRect.width - durationTextMetrics.boundingRect.x)

        horizontalAlignment: root.labelsInline ? Text.AlignLeft : Text.AlignRight
        verticalAlignment: Text.AlignVCenter

        TrackProgressWatchdog {
            id: durationWatchdog

            position: root.duration
        }
    }
}
