import QtQuick 1.1
import com.nokia.meego 1.1
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    objectName: "NewPostsPage"
    anchors.margins: UI.DEFAULT_MARGIN

    tools: forumTools

    property alias threads: threadListView.threads

    ThreadListView {
        id: threadListView

        title: "New Posts"
        showForum: true
    }

    ToolBarLayout {
        id: forumTools

        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }

        BusyToolIcon {
            anchors.centerIn: parent
            platformIconId: "toolbar-refresh"
            busy: forumSession.busy && threads.count > 0
            onClicked: threads.load(threads.firstPage)
        }
    }

    Component.onCompleted: {
        if (forumSession.sessionId) {
            threadListView.threads = forumSession.search("getnew")
        } else {
            forumSession.sessionIdChanged.connect(function() { threadListView.threads = forumSession.search("getnew") })
        }
    }
}
