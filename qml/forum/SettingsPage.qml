import QtQuick 1.1
import com.nokia.meego 1.1
import "../components" 1.0
import "../forum/UIConstants.js" as UI

Page {
    id: aboutPage
    anchors.margins: UI.DEFAULT_MARGIN
    orientationLock: PageOrientation.LockPortrait

    ViewHeader {
        id: header
        z: 1

        platformStyle: ViewHeaderStyle { titleImage: forumStyle.titleImage }
        text: "Settings"
    }

    Flickable {
        id: flick
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom }
        contentHeight: column.height + 2*UI.DEFAULT_MARGIN

        Column {
            id: column
            y: 2*UI.DEFAULT_MARGIN
            anchors { left: parent.left; right: parent.right }
            spacing: UI.DEFAULT_MARGIN

            CheckBox{
                text: "Dark Theme"
                checked: theme.inverted
                onClicked: {
                    theme.inverted = checked
                    themeSetting.value = checked
                }
            }
            CheckBox{
                text: "Show profile avatars"
                checked: dispAvatar
                onClicked: {
                    dispAvatar = checked
                    avatarSetting.value = checked
                }
            }
        }
    }

    ScrollDecorator {
        flickableItem: flick
        anchors.rightMargin: -UI.DEFAULT_MARGIN
    }

    tools: ToolBarLayout {
        visible: true
        ToolIcon {
            platformIconId: "toolbar-back";
            onClicked: pageStack.pop()
        }
    }
}
