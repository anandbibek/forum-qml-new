import QtQuick 1.1
import com.nokia.meego 1.0

Menu {
    MenuLayout {
        MenuItem {
            text: forumSession.sessionId ? "New posts" : "Today's posts"
            onClicked: {
                if (forumSession.sessionId) {
                    var threads = forumSession.search("getnew")
                    pageStack.push(Qt.createComponent("NewPostsPage.qml"), {"threads": threads, "title": text})
                } else {
                    var threads = forumSession.search("getdaily")
                    pageStack.push(Qt.createComponent("TodaysPostsPage.qml"), {"threads": threads, "title": text})
                }
            }
        }

        MenuItem {
            text: "Subscribed threads"
            visible: forumSession.sessionId
            onClicked: pageStack.push(Qt.createComponent("SubscriptionsPage.qml"))
        }

        MenuItem {
            text: "Open webpage"
            onClicked: Qt.openUrlExternally(forumSession.url)
        }
    }
}
