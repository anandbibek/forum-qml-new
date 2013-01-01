import QtQuick 1.1
import com.nokia.meego 1.1

Menu {
    id: root

    property QtObject post
    property Item parentPage

    MenuLayout {
        MenuItem {
            text: "Quote and reply"
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root.parentPage, {"post": post}).open()
        }

        MenuItem {
            text: post.thankedBy(forumSession.userName) ? "Remove your thanks" : "Thanks"
            onClicked: post.thankedBy(forumSession.userName) ? forumSession.unThank(post) : forumSession.thank(post)
        }
    }
}
