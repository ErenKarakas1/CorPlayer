import QtQuick
import QtQuick.Controls

ToolButton {
    id: root

    property string tooltipText

    display: AbstractButton.IconOnly

    opacity: {
        if (!enabled) return 0.7
        if (down) return 0.6
        if (hovered) return 1.0
        return 0.8
    }

    Behavior on opacity {
        NumberAnimation { duration: 100 }
    }

    ToolTip.visible: hovered && tooltipText.length > 0
    ToolTip.delay: 500
    ToolTip.text: tooltipText
}
