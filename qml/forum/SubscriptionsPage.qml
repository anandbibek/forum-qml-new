import QtQuick 1.1
import com.nokia.meego 1.1
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    objectName: "SubscriptionsPage"
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

        BusyToolIcon {
            anchors.centerIn: parent
            platformIconId: "toolbar-refresh"
            busy: forumSession.busy && threadListView.threads.count > 0
            onClicked: {
                if (forumSession.sessionId) {
                    threadListView.threads = forumSession.subscribedThreads()
                } else {
                    forumSession.sessionIdChanged.connect(function() { threadListView.threads = forumSession.subscribedThreads() })
                }
            }
        }

        ToolIcon {
            platformIconId: "toolbar-directory"
            onClicked: folderDialog.open()
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
