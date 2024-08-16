import QtQuick
import QtQuick.Controls

RoundButton {
    id: root

    property alias symbol: root.text
    font.family: "FontAwesome"

    radius: 100
}
