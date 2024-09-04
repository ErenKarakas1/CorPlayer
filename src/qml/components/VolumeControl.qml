import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: volumeControl

    property bool muted
    property alias volume: volumeSlider.value

    RowLayout {
        spacing: 0

        ToolButton {
            id: muteButton
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

            icon.name: volumeControl.muted ? "qrc:/icons/fa_volume_off" : "qrc:/icons/fa_volume_high"
            onClicked: volumeControl.muted = !volumeControl.muted
        }

        StepSlider {
            id: volumeSlider

            from: 0
            to: 100

            keyStepSize: 1
            wheelStepSize: 10
            enabled: !volumeControl.muted
        }
    }
}
