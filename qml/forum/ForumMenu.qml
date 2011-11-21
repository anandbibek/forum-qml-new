import QtQuick 1.1
import com.nokia.meego 1.0

Menu {
    id: root

    property QtObject forum

    MenuLayout {
        MenuItem {
            text: "Post a new thread"
            visible: forumSession.sessionId
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root.parent, {"forum": forum}).open()
        }

        MenuItem {
            text: "Mark forum read"
            visible: forumSession.sessionId
            onClicked: forum.markRead();
        }

        MenuItem {
            text: "Open webpage"
            onClicked: Qt.openUrlExternally(forumSession.externalUrl(forum.url))
        }
    }
}