import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import CorPlayer
import "../.."

GridLayout {
    id: root

    property int duration
    property color labelColor
    property bool labelsInline: true
    property bool playEnabled
    property int position
    property bool seekable

    signal seek(int position)

    columns: 5
    rowSpacing: 0
    rows: 2

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

        target: CorPlayer.trackPlaylistProxyModel
    }
    TextMetrics {
        id: durationTextMetrics
        text: "00:00:00"
    }

    Label {
        id: positionLabel

        Layout.alignment: Qt.AlignVCenter
        Layout.column: root.labelsInline ? 0 : 1
        Layout.fillHeight: true
        Layout.preferredWidth: (durationTextMetrics.boundingRect.width - durationTextMetrics.boundingRect.x)
        Layout.rightMargin: !root.labelsInline ? 0 : 10
        Layout.row: root.labelsInline ? 0 : 1
        color: root.labelColor
        horizontalAlignment: root.labelsInline ? Text.AlignRight : Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        text: timeWatchdog.progressDuration

        TrackProgressWatchdog {
            id: timeWatchdog
            position: root.position
        }
    }
    StepSlider {
        id: slider

        Layout.alignment: Qt.AlignVCenter
        Layout.column: 1
        Layout.columnSpan: 3
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.leftMargin: !root.labelsInline ? 0 : 10
        Layout.rightMargin: !root.labelsInline ? 0 : 10
        Layout.row: 0

        enabled: root.seekable && root.playEnabled

        // from, to and value of Slider are rescaled to seconds to avoid integer overflow issues
        from: 0
        keyStepSize: 5
        live: true
        to: root.duration / 1000
        wheelStepSize: 10

        onMoved: root.seek(value * 1000)
    }

    Label {
        id: durationLabel

        Layout.alignment: Qt.AlignVCenter
        Layout.column: root.labelsInline ? 4 : 3
        Layout.fillHeight: true
        Layout.leftMargin: !root.labelsInline ? 0 : 10
        Layout.preferredWidth: (durationTextMetrics.boundingRect.width - durationTextMetrics.boundingRect.x)
        Layout.row: root.labelsInline ? 0 : 1
        color: root.labelColor
        horizontalAlignment: root.labelsInline ? Text.AlignLeft : Text.AlignRight
        text: durationWatchdog.progressDuration
        verticalAlignment: Text.AlignVCenter

        TrackProgressWatchdog {
            id: durationWatchdog
            position: root.duration
        }
    }
}
