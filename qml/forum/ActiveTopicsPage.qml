import QtQuick 1.1
import com.nokia.meego 1.1
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    objectName: "ActiveTopicsPage"
    anchors.margins: UI.DEFAULT_MARGIN

    tools: forumTools

    ThreadListView {
        id: threadListView

        title: "Active Topics"
        threads: forumSession.activeTopics
        showForum: true
        longPress: false
        footerButton: Button {
                          anchors.horizontalCenter: parent.horizontalCenter
                          platformStyle: ButtonStyle { pressedBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-button-background-pressed" }
                          enabled: !forumSession.busy
                          text: "More..."
                          onClicked: {
                              pageStack.push(Qt.createComponent(forumSession.sessionId ? "NewPostsPage.qml" : "TodaysPostsPage.qml"),null,true)
                          }
                      }
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
            busy: forumSession.busy
            onClicked: forumSession.refresh()
        }
    }
}
