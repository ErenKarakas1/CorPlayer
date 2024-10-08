import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Button {
    id: root

    readonly property int diameter: parent.height

    Material.theme: Material.Dark
    height: diameter
    width: diameter

    // Define the background style for the button
    background: Rectangle {
        color: "white"
        border.color: "black"
        border.width: 2
        radius: diameter / 2
    }

    contentItem: Image {
        anchors.centerIn: parent
        source: root.icon.name
        sourceSize.width: diameter / 3
        sourceSize.height: diameter / 3
    }
}
