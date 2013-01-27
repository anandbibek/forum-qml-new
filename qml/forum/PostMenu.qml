import QtQuick 1.1
import com.nokia.meego 1.1

Menu {
    id: root

    property QtObject post
    property Item parentPage
    property int menuIndex

    MenuLayout {
        MenuItem {
            text: "Copy text"
            onClicked: {
                parentPage.copyIndex = index
                parentPage.bbCode = post.toBbCode();
            }
        }
        MenuItem {
            text: "Quote and reply"
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root.parentPage, {"post": post, "editPost": false}).open()
        }
        MenuItem {
            text: "Edit"
            visible: (forumSession.userName).toLowerCase() == (post.poster).toLowerCase()
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root.parentPage, {"post": post, "editPost": true}).open()
        }
        MenuItem {
            text: post.thankedBy(forumSession.userName) ? "Remove your thanks" : "Thanks"
            onClicked: post.thankedBy(forumSession.userName) ? forumSession.unThank(post) : forumSession.thank(post)
        }
        MenuItem {
            text: "Open webpage"
            onClicked: {
                Qt.openUrlExternally(forumSession.url + "/" + post.url)
                infoBanner.show()
            }
        }
    }
}
