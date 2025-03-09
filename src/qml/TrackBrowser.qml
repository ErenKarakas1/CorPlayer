pragma ComponentBehavior: Bound

import CorPlayer

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property var model
    property bool showHeader: true
    property bool dragEnabled: false
    property string headerTitle: ""
    property string headerText: ""

    signal trackDoubleClicked(int index)

    color: "transparent"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Optional header
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: root.showHeader

            Text {
                text: root.headerTitle
                color: "white"
                font.pixelSize: 48
                font.weight: Font.Bold
            }

            Text {
                text: root.headerText
                color: "#b3b3b3"
                font.pixelSize: 14
            }
        }

        // Tracks header
        Rectangle {
            Layout.fillWidth: true
            height: 32
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                spacing: 8

                Text {
                    Layout.preferredWidth: 48
                    text: "#"
                    color: "#b3b3b3"
                    font.pixelSize: 12
                }

                Text {
                    Layout.fillWidth: true
                    text: "Title"
                    color: "#b3b3b3"
                    font.pixelSize: 12
                }

                Text {
                    Layout.preferredWidth: 200
                    text: "Artist"
                    color: "#b3b3b3"
                    font.pixelSize: 12
                }

                Text {
                    Layout.preferredWidth: 200
                    text: "Album"
                    color: "#b3b3b3"
                    font.pixelSize: 12
                }

                Text {
                    Layout.preferredWidth: 64
                    text: "Duration"
                    color: "#b3b3b3"
                    font.pixelSize: 12
                }
            }
        }

        // Tracks list
        ListView {
            id: trackList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.model
            clip: true

            delegate: ItemDelegate {
                id: trackDelegate
                width: ListView.view.width
                height: 56

                required property int index
                required property string title
                required property string artist
                required property string album
                required property string durationString
                property bool isPlaying: false

                Drag.active: dragArea.drag.active
                Drag.dragType: Drag.Internal
                Drag.supportedActions: Qt.MoveAction
                Drag.proposedAction: Qt.MoveAction

                background: Rectangle {
                    color: trackDelegate.down || trackDelegate.highlighted
                        ? "#282828"
                        : (trackDelegate.hovered ? "#1a1a1a" : "transparent")
                }

                contentItem: RowLayout {
                    spacing: 8

                    Text {
                        Layout.preferredWidth: 48
                        text: trackDelegate.isPlaying
                            ? "♪"
                            : (trackDelegate.index + 1)
                        color: trackDelegate.isPlaying ? "#1db954" : "#b3b3b3"
                        font.pixelSize: 14
                    }

                    Text {
                        Layout.fillWidth: true
                        text: trackDelegate.title || "Unknown Title"
                        color: trackDelegate.isPlaying ? "#1db954" : "white"
                        font.pixelSize: 14
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: 200
                        text: trackDelegate.artist || ""
                        color: "#b3b3b3"
                        font.pixelSize: 14
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: 200
                        text: trackDelegate.album || ""
                        color: "#b3b3b3"
                        font.pixelSize: 14
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.preferredWidth: 64
                        text: trackDelegate.durationString || "--:--"
                        color: "#b3b3b3"
                        font.pixelSize: 14
                    }
                }

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    drag.target: root.dragEnabled ? parent : undefined

                    onDoubleClicked: {
                        root.trackDoubleClicked(trackDelegate.index)
                    }

                    onReleased: {
                        if (drag.active) {
                            const dropIndex = trackList.indexAt(
                                parent.x,
                                parent.y + parent.height / 2
                            )
                            if (dropIndex >= 0) {
                                CorPlayer.playlistProxyModel.moveRow(
                                    trackDelegate.index,
                                    dropIndex
                                )
                            }
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }
}
