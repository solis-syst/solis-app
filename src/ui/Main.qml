import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    width: 1280
    height: 800
    minimumWidth: 960
    minimumHeight: 640
    visible: true
    title: "Solis"

    palette.window: "#101114"
    palette.windowText: "#F2F3F5"
    palette.base: "#17191E"
    palette.text: "#F2F3F5"
    palette.button: "#1C1F26"
    palette.buttonText: "#F2F3F5"
    palette.highlight: "#3B82F6"
    palette.highlightedText: "#FFFFFF"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 220
            color: "#13151A"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                Label {
                    text: "SOLIS"
                    font.pixelSize: 24
                    font.bold: true
                    Layout.bottomMargin: 16
                }

                Repeater {
                    model: ["Engine", "Visual", "Coach", "Puzzles", "Misc"]

                    delegate: Button {
                        Layout.fillWidth: true
                        text: modelData
                        flat: true
                        horizontalAlignment: Text.AlignLeft

                        onClicked: contentTitle.text = modelData
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                Label {
                    text: "Native Desktop"
                    opacity: 0.6
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#101114"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 18

                Label {
                    id: contentTitle
                    text: "Engine"
                    font.pixelSize: 28
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    radius: 12
                    color: "#17191E"
                    border.color: "#242832"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 10

                        Label {
                            text: "Solis Analyzer"
                            font.pixelSize: 20
                            font.bold: true
                        }

                        Label {
                            text: "Native C++ engine and managed-browser backend."
                            opacity: 0.7
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }
}
