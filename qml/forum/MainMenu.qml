import QtQuick 1.1
import com.nokia.meego 1.1

Menu {
    MenuLayout {
        MenuItem {
            text: forumSession.sessionId ? "New posts" : "Today's posts"
            onClicked: pageStack.push(Qt.createComponent(forumSession.sessionId ? "NewPostsPage.qml" : "TodaysPostsPage.qml"))
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

        MenuItem {
            text: "About"
            onClicked: pageStack.push(Qt.createComponent("AboutPage.qml"))
        }
    }
}
