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
    }

    ListView {
        id: threadList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: UI.DEFAULT_MARGIN
        anchors.bottom: parent.bottom
        cacheBuffer: contentHeight

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
            platformStyle: ButtonStyle { pressedBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-button-background-pressed" }
            visible: !!threads && threads.count > 0 && threads.lastPage < threads.numPages
            enabled: !forumSession.busy
            text: "Older threads"
            onClicked: threads.load(threads.lastPage + 1)
        }
    }

    FastScroll {
        listView: threadList
        visible: !!threads && threads.count > 0
    }

    BusyIndicator {
        anchors.centerIn: threadList
        platformStyle: BusyIndicatorStyle { size: "large" }
        running: forumSession.busy
        visible: forumSession.busy && (!threads || threads.count == 0)
    }

    function openThread(thread, openMode) {
        if (openMode)
            thread.openMode = openMode

        pageStack.push(Qt.createComponent("ThreadPage.qml"), {"thread": thread});
    }
}
