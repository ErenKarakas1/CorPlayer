import QtQuick
import QtQuick.Controls

ToolButton {
    id: root

    property int size: 36
    property string tooltipText

    display: AbstractButton.IconOnly

    width: size
    height: size
    padding: 8

    contentItem: Image {
        source: root.icon.name
        sourceSize: Qt.size(15, 24)
        fillMode: Image.PreserveAspectFit
        anchors.centerIn: parent
    }

    opacity: {
        if (!enabled) return 0.7
        if (down) return 0.6
        if (hovered) return 0.8
        return 1.0
    }

    background: Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "white"

        Behavior on color {
            ColorAnimation { duration: 100 }
        }
    }

    ToolTip.visible: hovered && tooltipText.length > 0
    ToolTip.delay: 500
    ToolTip.text: tooltipText
}
