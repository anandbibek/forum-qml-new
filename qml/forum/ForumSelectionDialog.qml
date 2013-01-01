import QtQuick 1.1
import com.nokia.meego 1.1
import Forum 1.0
import "UIConstants.js" as UI

SelectionDialog {
    id: root
    titleText: "Select forum"

    platformStyle: SelectionDialogStyle { itemHeight: 80 }

    delegate: Item {
        id: delegateItem
        property bool selected: index == selectedIndex;

        height: root.platformStyle.itemHeight;
        anchors.left: parent.left
        anchors.right: parent.right

        MouseArea {
            id: delegateMouseArea
            anchors.fill: parent;
            onPressed: selectedIndex = index;
            onClicked: accept();
        }

        Rectangle {
            id: backgroundRect
            anchors.fill: parent
            color: delegateItem.selected ? root.platformStyle.itemSelectedBackgroundColor : root.platformStyle.itemBackgroundColor
        }

        BorderImage {
            id: background
            anchors.fill: parent
            border { left: UI.CORNER_MARGINS; top: UI.CORNER_MARGINS; right: UI.CORNER_MARGINS; bottom: UI.CORNER_MARGINS }
            source: delegateMouseArea.pressed ? root.platformStyle.itemPressedBackground :
                    delegateItem.selected ? root.platformStyle.itemSelectedBackground :
                    root.platformStyle.itemBackground
        }

        Text {
            id: itemText
            elide: Text.ElideRight
            color: delegateItem.selected ? root.platformStyle.itemSelectedTextColor : root.platformStyle.itemTextColor
            anchors.verticalCenter: delegateItem.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: root.platformStyle.itemLeftMargin
            anchors.rightMargin: root.platformStyle.itemRightMargin
            text: name;
            font: root.platformStyle.itemFont
        }

        Image {
            source: iconName ? ("image://theme/" + iconName) : ""
            anchors.right: parent.right
            anchors.rightMargin: UI.DEFAULT_MARGIN
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    model: ForumProviderList { }

    onAccepted: forumSession.provider = model.get(selectedIndex).provider
}
