import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CorPlayer

import "components"

FocusScope {
    id: volumeControl

    property bool isMuted: CorPlayer.mediaPlayer.isMuted

    signal changeMute

    onChangeMute: CorPlayer.mediaPlayer.setMuted(!isMuted)

    RowLayout {
        id: volumeControlLayout
        anchors.fill: parent
        spacing: 5

        ToolButton {
            id: muteButton
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

            icon.name: isMuted ? "qrc:/icons/fa_volume_off" : "qrc:/icons/fa_volume_high"

            onClicked: volumeControl.changeMute();
        }

        VolumeSlider {
            id: volumeSlider

            Layout.preferredWidth : parent.width*0.1
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

            //volume: playbackControl.volume
            //position: playbackControl.volume

            //onChangeVolume: playbackControl.changeVolume(position)
        }
    }
}
