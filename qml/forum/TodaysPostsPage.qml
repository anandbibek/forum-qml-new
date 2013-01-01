import QtQuick 1.1
import com.nokia.meego 1.1
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    objectName: "TodaysPostsPage"
    anchors.margins: UI.DEFAULT_MARGIN

    tools: forumTools

    property alias threads: threadListView.threads

    ThreadListView {
        id: threadListView

        title: "Today's Posts"
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

    Component.onCompleted: threads = forumSession.search("getdaily")
}
