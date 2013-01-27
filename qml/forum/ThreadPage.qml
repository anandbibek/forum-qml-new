import QtQuick 1.1
import com.nokia.meego 1.1
import Forum 1.0
import "../components"
import "../components/UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    property QtObject thread
    property int copyIndex : -1
    property string bbCode : ""

    ListView {
        id: postList

        anchors.fill: parent
        cacheBuffer: 10*height
        //pressDelay: 100

        section.delegate: SectionHeader { }
        section.property: "section"

        delegate: PostDelegate {
            copyEnabled: (copyIndex === index)
            modelBody: bbCode
            onClicked: {
                copyIndex = -1
                bbCode = ""
                //console.debug(thread.model.get(index).toBbCode())
            }
            onPressAndHold: {
                if (forumSession.sessionId)
                    Qt.createComponent("PostMenu.qml").createObject(root, {"post": thread.model.get(index), "parentPage": root, "menuIndex": index}).open()
            }
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
                platformStyle: ButtonStyle { pressedBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-button-background-pressed" }

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
                platformStyle: ButtonStyle { pressedBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-button-background-pressed" }

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

    FastScroll {
        listView: postList
        platformStyle : FastScrollStyle{railImage: "image://theme/meegotouch-fast-scroll-rail"}
        visible: !!thread
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
            onClicked: pageStack.pop()
        }

        ToolIcon {
            platformIconId: "toolbar-add"
            visible: forumSession.sessionId
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root, {"thread": thread, "editPost": false}).open()
        }

        BusyToolIcon {
            platformIconId: "toolbar-refresh"
            busy: forumSession.busy && thread.model.count > 0
            onClicked: thread.model.load(thread.model.lastPage)
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
                text: thread.subscribed ? "Unsubscribe" : "Subscribe"
                visible: forumSession.sessionId
                onClicked: thread.subscribed ? forumSession.unsubscribe(thread) : Qt.createComponent("EmailNotification.qml").createObject(root, {"thread": thread}).open()
                //forumSession.subscribe(thread)
            }

            MenuItem {
                text: "First page"
                visible: thread.model.firstPage > 1
                onClicked: thread.model.load(1)
            }

            MenuItem{
                visible: thread.model.count > 1
                text: "Jump to page"
                onClicked: Qt.createComponent("PageJump.qml").createObject(root, {"thread": thread})
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
                    infoBanner.show()
                    if((thread.model.url).indexOf("http")>=0)
                        Qt.openUrlExternally(thread.model.url)
                    else
                        Qt.openUrlExternally(forumSession.url + "/" + thread.model.url)
                }
            }
        }
    }

    // Make sure the thread doesn't get deleted while the thread page is opened
    Component.onCompleted: thread.take()
    Component.onDestruction: thread.release()
}
