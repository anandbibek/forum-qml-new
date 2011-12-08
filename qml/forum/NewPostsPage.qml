import QtQuick 1.1
import com.nokia.meego 1.0
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    tools: forumTools

    property alias title: threadListView.title
    property alias threads: threadListView.threads

    ThreadListView {
        id: threadListView

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
}
