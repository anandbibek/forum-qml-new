import QtQuick 1.1
import com.nokia.meego 1.0
import Forum 1.0
import "UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    property QtObject forum

    ThreadListView {
        id: threadListView

        title: forum.title
        threads: forum.model
    }

    tools: ToolBarLayout {
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }

        ToolIcon {
            platformIconId: "toolbar-add"
            visible: forumSession.sessionId
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root, {"forum": forum}).open()
        }

        ToolIcon {
            platformIconId: busyIndicator.visible ? "" : "toolbar-refresh"
            onClicked: forum.model.load(forum.model.firstPage)

            BusyIndicator {
                id: busyIndicator
                anchors.centerIn: parent
                running: visible
                visible: forumSession.busy && forum.model.count > 0
            }
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: Qt.createComponent("ForumMenu.qml").createObject(root, {"forum": forum}).open()
        }
    }

 // Component.onCompleted: forum.maybeUpdate()
}
