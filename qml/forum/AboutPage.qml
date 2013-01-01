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
        text: forumStyle.titleText + " 0.1.0"
    }

    Flickable {
        id: flick
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom }
        contentHeight: column.height + UI.DEFAULT_MARGIN

        Column {
            id: column
            y: UI.DEFAULT_MARGIN
            anchors { left: parent.left; right: parent.right }
            spacing: UI.DEFAULT_MARGIN

            Label {
                platformStyle: labelStyle
                anchors { left: parent.left; right: parent.right }

                color: UI.COLOR_SECONDARY_FOREGROUND
                horizontalAlignment: Text.Center
                text: "Copyright (c) 2011 Philipp Zabel<br />" +
                      "&lt;<a href='mailto:Philipp Zabel <philipp.zabel@gmail.com>' style='color:" + UI.COLOR_SECONDARY_FOREGROUND + "'>philipp.zabel@gmail.com</a>&gt;"
                + "<br />Copyright (c) 2013 Anand Bibek<br />" +
                      "&lt;<a href='mailto:Anand Bibek <ananda.bibek@gmail.com>' style='color:" + UI.COLOR_SECONDARY_FOREGROUND + "'>ananda.bibek@gmail.com</a>&gt;"
                onLinkActivated: Qt.openUrlExternally(link)
            }

            Label {
                platformStyle: labelStyle
                width: parent.width
                wrapMode: Text.WordWrap
                text: forumStyle.legal
                onLinkActivated: Qt.openUrlExternally(link)
            }

            LabelStyle {
                id: labelStyle
                fontPixelSize: UI.FONT_LSMALL
                fontFamily: UI.FONT_FAMILY_LIGHT
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
