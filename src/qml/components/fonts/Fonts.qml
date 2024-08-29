pragma Singleton

import QtQuick

Item {
    property alias fontAwesome: fontLoader.name

    FontLoader {
        id: fontLoader
        source: "qrc:/fonts/FontAwesome.otf"
    }
}
