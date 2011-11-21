import QtQuick 1.1
import com.nokia.meego 1.0

Menu {
    id: root

    property QtObject post

    MenuLayout {
        MenuItem {
            text: "Quote and reply"
            visible: forumSession.sessionId
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root.parent, {"post": post}).open()
        }

        MenuItem {
            text: post.thankedBy(forumSession.userName) ? "Remove your thanks" : "Thanks"
            visible: forumSession.sessionId
            onClicked: post.thankedBy(forumSession.userName) ? forumSession.unThank(post) : forumSession.thank(post)
        }
    }
}
