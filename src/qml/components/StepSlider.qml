import QtQuick
import QtQuick.Controls

Slider {
    id: slider

    property real keyStepSize: stepSize
    property real wheelStepSize: stepSize
    
    readonly property real __keyStepSize: keyStepSize === null ? 0 : keyStepSize
    readonly property real __wheelStepSize: wheelStepSize === null ? 0 : wheelStepSize

    function __move(step: real) {
        value = Math.max(from, Math.min(value + step, to));
        moved();
    }

    Accessible.onDecreaseAction: __move(-__keyStepSize)
    Accessible.onIncreaseAction: __move(__keyStepSize)
    Keys.onLeftPressed: __move(-__keyStepSize)
    Keys.onRightPressed: __move(__keyStepSize)

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: (wheel) => {
            return slider.__move(slider.__wheelStepSize * (wheel.angleDelta.y > 0 ? 1 : -1));
        }
    }

    background: Rectangle {
        x: slider.leftPadding
        anchors.verticalCenter: parent.verticalCenter

        implicitWidth: parent.width
        implicitHeight: 4
        width: slider.availableWidth
        height: implicitHeight

        radius: 5
        color: "gray"

        Rectangle {
            width: slider.visualPosition * parent.width
            height: parent.height

            radius: 5
            color: "white"
        }
    }

    handle: Rectangle {
        x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
        anchors.verticalCenter: parent.verticalCenter

        width: 16
        height: 16

        radius: width / 2
        color: "white"
    }
}
