import QtQuick 1.1
import com.nokia.meego 1.0
import Forum 1.0
import "../components"
import "../components/UIConstants.js" as UI

Item {
    id: root
    anchors.fill: parent

    property QtObject threads
    property alias title: header.text
    property bool showForum: false

    ViewHeader {
        id: header
        z: 1

        BusyIndicator {
            anchors.right: parent.right
            anchors.rightMargin: UI.DEFAULT_MARGIN
            anchors.verticalCenter: parent.verticalCenter
            platformStyle: BusyIndicatorStyle { inverted: true }
            running: forumSession.busy
            visible: forumSession.busy && threads.count > 0
        }
    }

    ListView {
        id: threadList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: UI.DEFAULT_MARGIN
        anchors.bottom: parent.bottom

        section.delegate: SectionHeader { }
        section.property: "section"

        delegate: ThreadDelegate {
            platformStyle: ThreadDelegateStyle { showForum: root.showForum }
            onClicked: openThread(threads.get(index))
            onPressAndHold: Qt.createComponent("ThreadMenu.qml").createObject(root, {thread: threads.get(index)}).open()
        }
        model: threads

        footer: Button {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: threads.count > 0 && threads.lastPage < threads.numPages
            enabled: !forumSession.busy
            text: "Older threads"
            onClicked: threads.load(threads.lastPage + 1)
        }
    }

    FastScroll {
        listView: threadList
        visible: threads != null && threads.count > 0
    }

    BusyIndicator {
        anchors.centerIn: threadList
        platformStyle: BusyIndicatorStyle { size: "large" }
        running: forumSession.busy
        visible: forumSession.busy && threads.count == 0
    }

    function openThread(thread, openMode) {
        if (openMode)
            thread.openMode = openMode

        // Detach the PostList with a copy of the Thread in case the thread in the list
        // is destroyed while PostListPage is still opened (which happens for Active Topics)

        pageStack.push(Qt.createComponent("ThreadPage.qml"), {"thread": thread.detachWithModel()});
    }
}
