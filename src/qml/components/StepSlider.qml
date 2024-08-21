import QtQuick
import QtQuick.Controls

Slider {
    id: root

    property real keyStepSize: stepSize
    property real wheelStepSize: stepSize

    readonly property real __keyStepSize: keyStepSize === null ? 0 : keyStepSize
    readonly property real __wheelStepSize: wheelStepSize === null ? 0 : wheelStepSize

    function __move(step: real) {
        value = Math.max(from, Math.min(value + step, to))
        moved()
    }

    Accessible.onDecreaseAction: __move(-__keyStepSize)
    Accessible.onIncreaseAction: __move(__keyStepSize)
    Keys.onLeftPressed: __move(-__keyStepSize)
    Keys.onRightPressed: __move(__keyStepSize)

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: wheel => root.__move(root.__wheelStepSize * (wheel.angleDelta.y > 0 ? 1 : -1))
    }
}
