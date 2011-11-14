import QtQuick 1.1
import com.nokia.meego 1.0
import Forum 1.0

Menu {
    id: root

    property QtObject thread

    MenuLayout {
        MenuItem {
            text: "First post"
            onClicked: openThread(root.thread, Thread.FirstPost)
        }

        MenuItem {
            text: "First unread post"
            visible: forumSession.sessionId
            onClicked: openThread(root.thread, Thread.FirstUnreadPost)
        }

        MenuItem {
            text: "Last post"
            onClicked: openThread(root.thread, Thread.LastPost)
        }

        MenuItem {
            text: "Open webpage"
            onClicked: {
                console.log(root.thread.url);
                Qt.openUrlExternally(forumSession.url + "/" + root.thread.url)
            }
        }
    }

    function openThread(thread, openMode) {
        if (openMode)
            thread.openMode = openMode

        // Detach the PostList with a copy of the Thread in case the thread in the list
        // is destroyed while PostListPage is still opened (which happens for Active Topics)
        pageStack.push(Qt.createComponent("ThreadPage.qml"), {"thread": thread.detachWithModel()});
    }
}
