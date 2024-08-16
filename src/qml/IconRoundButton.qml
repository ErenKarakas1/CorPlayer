import QtQuick
import QtQuick.Controls

import "./fonts/"

RoundButton {
    id: root

    property alias symbol: root.text

    font.family: Fonts.faWeb
}
