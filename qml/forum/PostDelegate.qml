import QtQuick 1.0
import com.nokia.meego 1.0
import "../components/constants.js" as UI

Item {
    id: root

    signal clicked
    signal pressAndHold
    property alias pressed: mouseArea.pressed

    property int titleSize: UI.LIST_TILE_SIZE
    property int titleWeight: Font.Bold
    property color titleColor: theme.inverted ? UI.LIST_TITLE_COLOR_INVERTED : UI.LIST_TITLE_COLOR

    property int bodySize: UI.LIST_SUBTILE_SIZE
    property int bodyWeight: Font.Light
    property color bodyColor: theme.inverted ? UI.LIST_SUBTITLE_COLOR_INVERTED : UI.LIST_SUBTITLE_COLOR

    height: column.height + 3 * UI.MARGIN_XLARGE
    width: parent.width

    BorderImage {
        id: background
        anchors { fill: parent; topMargin: UI.MARGIN_XLARGE / 2; bottomMargin: UI.MARGIN_XLARGE / 2 }

        source: "image://theme/meegotouch-list-background"
        border { left: 21; right: 21; top: 21; bottom: 21 }
    }

    MouseArea {
        id: mouseArea;
        anchors.fill: parent
        onClicked: root.clicked()
        onPressAndHold: root.pressAndHold()
    }

    Column {
        id: column

        anchors { top: parent.top; left: parent.left; right: parent.right;
                  topMargin: 1.5 * UI.MARGIN_XLARGE }
        height: poster.height + UI.MARGIN_XLARGE + body.height
        spacing: UI.MARGIN_XLARGE

        Item {
            anchors { left: parent.left; right: parent.right;
                      leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }
            height: poster.height

            Label {
                id: poster
                text: model.poster
                font.family: UI.FONT_FAMILY
                font.pixelSize: UI.FONT_LIGHT_SIZE
                color: UI.LIST_SUBTITLE_COLOR
            }
            Label {
                id: dateTime
                text: model.dateTime
                anchors.right: parent.right
                font.family: UI.FONT_FAMILY_LIGHT
                font.pixelSize: 18
                color: UI.LIST_SUBTITLE_COLOR
            }
        }

        Label {
            id: body
            anchors { left: parent.left; right: parent.right;
                      leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }

            text: model.body
            font.family: UI.FONT_FAMILY_LIGHT
            font.pixelSize: UI.FONT_LIGHT_SIZE
            clip: true

            onLinkActivated: {
                console.log("Clicked on link:" + link)
                Qt.openUrlExternally(link);
            }
        }
    }
}
