import QtQuick 1.1

Image {
    id: root
    anchors { left: parent.left; right: parent.right }
    height: 48

    property bool expanded
    property alias pressed: mouseArea.pressed

    source: "image://theme/meegotouch-recipient-divider-white-bottom"
//  source: "image://theme/meegotouch-recipient-divider-neutral"

    Image {
        anchors.centerIn: parent
        source: "image://theme/meegotouch-recipient-divider-" + (expanded ? "collapse" : "expand") + (pressed ? "-pressed" : "")
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: expanded = !expanded
    }
}
