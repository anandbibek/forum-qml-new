import QtQuick 1.1
import com.nokia.meego 1.0
import Forum 1.0
import "../components"
import "../components/UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    property QtObject thread

    ListView {
        id: postList

        anchors.fill: parent

        section.delegate: SectionHeader { }
        section.property: "section"

        delegate: PostDelegate {
            onClicked: {
                console.debug(thread.model.get(index).toBbCode())
            }
            onPressAndHold: Qt.createComponent("PostMenu.qml").createObject(root, {"post": thread.model.get(index), "thread": thread}).open()
        }

        model: thread.model

        header: Item {
            width: postList.width
            height: viewHeader.height + (olderButton.visible ? olderButton.height : 0) - UI.MARGIN_XLARGE

            ThreadViewHeader {
                id: viewHeader
                text: thread ? thread.title : ""
                subtitle: (thread && thread.forum)
                          ? ("in <span style='color:" + forumStyle.highlightTextColor + "'>" + thread.forum + "</span>")
                          : ""
            }

            Button {
                id: olderButton
                anchors.top: viewHeader.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                enabled: !forumSession.busy
                visible: thread.model.count > 0 && thread.model.firstPage > 1
                text: "Older posts"
                onClicked: thread.model.load(thread.model.firstPage - 1)
            }
        }

        footer: Item {
            width: postList.width
            height: newerButton.visible ? (newerButton.height + UI.MARGIN_XLARGE / 2) : 0

            Button {
                id: newerButton
                y: UI.MARGIN_XLARGE / 2
                anchors.horizontalCenter: parent.horizontalCenter

                enabled: !forumSession.busy
                visible: thread.model.count > 0 && thread.model.lastPage < thread.model.numPages
                text: "Newer posts"
                onClicked: thread.model.load(thread.model.lastPage + 1)
            }
        }

        /*
        NumberAnimation on contentY {
            id: scroll
            running: false
            duration: 500
            easing.type: Easing.InOutExpo
        }
        */
    }

    Connections {
        target: thread.model
        onRowsInserted: {
            if (postList.contentY == 0)
                postList.positionViewAtBeginning()
            if (thread.model.jumpToIndex > 0) {
                console.log("Rows inserted, jumping to index " + thread.model.jumpToIndex)
                // FIXME - Smooth scrolling list views does not work properly.
                // contentY jumps around wildly when delegates of different height appear in the view.
                /*
                scroll.from = postList.contentY
                if (thread.model.jumpToIndex == thread.model.count - 1)
                    postList.positionViewAtEnd()
                else
                    postList.positionViewAtIndex(thread.model.jumpToIndex, ListView.Beginning)
                scroll.to = postList.contentY
                postList.positionViewAtBeginning()
                scroll.start()
                */
                if (thread.model.jumpToIndex == thread.model.count - 1)
                    postList.positionViewAtEnd()
                else
                    postList.positionViewAtIndex(thread.model.jumpToIndex, ListView.Beginning)
            }
        }
    }

    ScrollDecorator {
        flickableItem: postList
    }

    BusyIndicator {
        anchors.centerIn: postList
        platformStyle: BusyIndicatorStyle { size: "large" }
        running: visible
        visible: forumSession.busy && thread.model.count == 0
    }

    tools: ToolBarLayout {
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: {
                thread.destroy(250) // delay until page finished sliding out
                pageStack.pop()
            }
        }

        ToolIcon {
            platformIconId: "toolbar-add"
            visible: !busyIndicator.visible && forumSession.sessionId
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root, {"thread": thread}).open()
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            running: visible
            visible: forumSession.busy && thread.model.count > 0
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: postListMenu.open()
        }
    }

    Menu {
        id: postListMenu
        MenuLayout {
            MenuItem {
                text: "First page"
                visible: thread.model.firstPage > 1
                onClicked: thread.model.load(1)
            }
            MenuItem {
                text: "Last page"
                visible: thread.model.lastPage < thread.model.numPages
                onClicked: thread.model.load(thread.model.numPages)
            }
            MenuItem {
                text: "Open webpage"
                onClicked: {
                    console.log(thread.model.url)
                    Qt.openUrlExternally(thread.model.url)
                }
            }
        }
    }
}