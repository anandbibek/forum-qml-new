import QtQuick 1.1
import com.nokia.meego 1.0
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    tools: forumTools

    property alias threads: threadListView.threads

    ThreadListView {
        id: threadListView

        title: "Active Topics"
        showForum: true
    }

    ToolBarLayout {
        id: forumTools

        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }

        ToolIcon {
            platformIconId: "toolbar-refresh"
            anchors.centerIn: parent
            onClicked: forumSession.refresh()
        }
    }
}
