import QtQuick 1.0
import com.nokia.meego 1.1
import "../components/constants.js" as UI

Item {
    id: listItem

    signal clicked
    signal pressAndHold
    property alias pressed: mouseArea.pressed

    property int titleSize: UI.LIST_TILE_SIZE
    property int titleWeight: Font.Bold
    property color titleColor: forumStyle.forumTitleTextColor

    property int subtitleSize: UI.LIST_SUBTILE_SIZE
    property color subtitleColor: theme.inverted ? UI.LIST_SUBTITLE_COLOR_INVERTED : UI.LIST_SUBTITLE_COLOR

    height: column.height +  UI.LIST_ITEM_SPACING
    width: parent.width

    BorderImage {
        id: background
        anchors.fill: parent
        // Fill page porders
        anchors.leftMargin: -UI.MARGIN_XLARGE
        anchors.rightMargin: -UI.MARGIN_XLARGE
        visible: mouseArea.pressed
        source: "image://theme/meegotouch-list-fullwidth-background-pressed-center"
    }

    Column {
        id:column
        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }

        Label {
            id: mainText
            text: model.title
            font.weight: listItem.titleWeight
            font.pixelSize: listItem.titleSize
            color: listItem.titleColor

            anchors.left: parent.left
            anchors.right: parent.right
            elide: Text.ElideRight
        }

        Label {
            id: subText
            text: model.subtitle
            font.family: UI.FONT_FAMILY_LIGHT
            font.pixelSize: listItem.subtitleSize
            color: listItem.subtitleColor

            visible: text != ""
            anchors.left: parent.left
            anchors.right: parent.right
            elide: Text.ElideRight
            maximumLineCount: 2
        }
    }

    MouseArea {
        id: mouseArea;
        anchors.fill: parent
        onClicked: listItem.clicked()
        onPressAndHold: listItem.pressAndHold()
    }
}
