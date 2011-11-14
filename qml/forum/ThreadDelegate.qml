import QtQuick 1.0
import com.nokia.meego 1.0
import com.nokia.extras 1.0
import "../components/constants.js" as UI

Item {
    id: root

    signal clicked
    signal pressAndHold
    property alias pressed: mouseArea.pressed

    property Style platformStyle: ThreadDelegateStyle { }

    height: UI.LIST_ITEM_HEIGHT
    width: parent.width

    BorderImage {
        id: background
        anchors.fill: parent
        border { left: UI.CORNER_MARGINS; top: UI.CORNER_MARGINS; bottom: UI.CORNER_MARGINS; right: UI.CORNER_MARGINS }
        // Fill page borders
        anchors.leftMargin: -UI.MARGIN_XLARGE
        anchors.rightMargin: -UI.MARGIN_XLARGE
        visible: mouseArea.pressed || (root.platformStyle.showUnread && model.unread)
        source: (root.platformStyle.showUnread && model.unread) ?
                ("image://theme/" + forumStyle.colorThemeString + "meegotouch-unread-inbox-panel-background" + (mouseArea.pressed ? "-pressed" : "")) :
                "image://theme/meegotouch-list-fullwidth-background-pressed-center"
    }

    Column {
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        anchors.leftMargin: root.platformStyle.showUnread ? 13 : 0 // 13px is the same as in the email program

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: mainText.height

            Label {
                id: mainText
                anchors.left: parent.left
                anchors.right: attachmentIcon.visible ? attachmentIcon.left : repliesBubble.left

                color: root.platformStyle.titleColor
                font.pixelSize: root.platformStyle.titleSize
                font.weight: root.platformStyle.titleWeight
                text: model.title
                elide: Text.ElideRight
            }

            Image {
                id: attachmentIcon
                anchors.right: repliesBubble.left
                anchors.rightMargin: 8

                visible: model.attachments > 0
                source: "image://theme/icon-s-email-attachment"
            }

            CountBubble {
                id: repliesBubble
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                value: model.replies
            }
        }

        RatingIndicator {
            id: ratingIndicator
            visible: model.votes >= 0
            ratingValue: model.ratingValue
            maximumValue: 5
            count: model.votes
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: subText.height

            Label {
                id: subText
                anchors.left: parent.left
                anchors.right: timeLabel.left
                height: ratingIndicator.visible ? font.pixelSize : undefined

                color: root.platformStyle.subtitleColor
                font.family: UI.FONT_FAMILY_LIGHT
                font.pixelSize: root.platformStyle.subtitleSize
                visible: text != ""
                text: ((root.platformStyle.showForum && model.forum) ?
                      ("to <span style='color:" + forumStyle.highlightTextColor + "'>" + model.forum + "</span> by&nbsp;<span>") :
                      ("by <span style='color:" + forumStyle.highlightTextColor + "'>")) + model.poster + "</span>"
                elide: Text.ElideRight
            }

            Label {
                id: timeLabel
                font.family: UI.FONT_FAMILY_LIGHT
                font.pixelSize: root.platformStyle.subtitleSize
                color: root.platformStyle.subtitleColor

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                text: model.dateTime
            }
        }
    }

    MouseArea {
        id: mouseArea;
        anchors.fill: parent
        onClicked: root.clicked();
        onPressAndHold: root.pressAndHold();
    }
}
