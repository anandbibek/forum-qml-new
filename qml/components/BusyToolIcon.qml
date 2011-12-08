import QtQuick 1.1
import com.nokia.meego 1.0

Item {
    id: root
    property alias iconSource: toolIcon.iconSource
    property alias platformIconId: toolIcon.platformIconId
    property bool busy

    width: toolIcon.width
    height: toolIcon.height
    signal clicked

    ToolIcon {
        id: toolIcon
        onClicked: root.clicked()
        visible: !busy
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: visible
        visible: busy
    }
}
