import QtQuick 1.1
import com.nokia.meego 1.0
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    ViewHeader {
        id: header
        z: 1

        platformStyle: ViewHeaderStyle { titleImage: forumStyle.titleImage }
        text: forumStyle.titleText

        BusyIndicator {
            anchors.right: parent.right
            anchors.rightMargin: UI.DEFAULT_MARGIN
            anchors.verticalCenter: parent.verticalCenter
            platformStyle: BusyIndicatorStyle { inverted: true }
            running: visible
            visible: forumSession.busy && forumList.count > 0
        }
    }

    ListView {
        id: forumList

        property bool hasSections: false

        anchors { left: parent.left; right: parent.right }
        anchors.top: header.bottom
        anchors.topMargin: UI.DEFAULT_MARGIN
        anchors.bottom: parent.bottom

        section.delegate: SectionHeader { }
        section.property: "section"

        header: Column {
            width: parent.width
            spacing: UI.DEFAULT_MARGIN
            visible: forumSession.missingCredentials
            height: visible ? childrenRect.height : 0

            Text {}
            Label {
                width: parent.width

                color: UI.COLOR_SECONDARY_FOREGROUND
                horizontalAlignment: Text.AlignHCenter
                text: "Login to post messages and follow subscribed threads"
            }

            Button {
                id: loginButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Login"
                onClicked: {
                    if (forumSession.canSignOn)
                        forumSession.signOn()
                    else
                        pageStack.push(Qt.createComponent("LoginPage.qml"))
                }
            }
        }

        delegate: ForumDelegate {
            onClicked: {
                var forum = forumSession.forums.get(index)
                if (forum.url)
                    pageStack.push(Qt.createComponent("ForumPage.qml"), {"forum": forum});
            }
            onPressAndHold: Qt.createComponent("ForumMenu.qml").createObject(root, {"forum": forumSession.forums.get(index)}).open()
        }
        model: forumSession.forums

        Connections {
            target: forumSession.forums
            onRowsInserted: forumList.hasSections = forumSession.forums.get(0).section
        }

        Component.onCompleted: forumList.hasSections = model.count > 0 && model.get(0).section
    }

    FastScroll {
        listView: forumList
        visible: forumList.hasSections
    }

    ScrollDecorator {
        flickableItem: forumList
        visible: !forumList.hasSections
    }

    BusyIndicator {
        anchors.centerIn: forumList
        platformStyle: BusyIndicatorStyle { size: "large" }
        running: visible
        visible: forumSession.busy && forumList.count == 0
    }

    tools: ToolBarLayout {
        ToolIcon {
            platformIconId: "toolbar-add"
            opacity: 0
            enabled: false
            onClicked: Qt.createComponent("NewPostSheet.qml").createObject(root).open()
        }

        ToolIcon {
            platformIconId: "toolbar-history"
            opacity: forumSession.activeTopics != null ? 1 : 0
            onClicked: pageStack.push(Qt.createComponent("ActiveTopicsPage.qml"), {"threads": forumSession.activeTopics})
        }

        ToolIcon {
            platformIconId: "toolbar-search"
            onClicked: pageStack.push(Qt.createComponent("SearchPage.qml"))
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: Qt.createComponent("MainMenu.qml").createObject(root).open()
        }
    }
}
