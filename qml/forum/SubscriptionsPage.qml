import QtQuick 1.1
import com.nokia.meego 1.0
import "UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    ThreadListView {
        id: threadListView

        showForum: true
        title: "Subscriptions"
    }

    tools: ToolBarLayout {
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: {
                if (threadListView.threads)
                    threadListView.threads.destroy(250)
                pageStack.pop()
            }
        }

        ToolIcon {
            platformIconId: "toolbar-directory"
            onClicked: folderDialog.open()
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu"
            visible: false
        }
    }

    SelectionDialog {
        id: folderDialog

        titleText: "Select folder"
        model: ListModel {
            ListElement { name: "Subscriptions" }
        }

        Component.onCompleted: selectedIndex = 0
    }

    Component.onCompleted: {
        if (forumSession.sessionId) {
            threadListView.threads = forumSession.subscribedThreads()
        } else {
            forumSession.sessionIdChanged.connect(function() { threadListView.threads = forumSession.subscribedThreads() })
        }
    }
}
