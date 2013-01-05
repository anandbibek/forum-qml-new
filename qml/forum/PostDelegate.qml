import QtQuick 1.0
import com.nokia.meego 1.1
import QtWebKit 1.0
import "../components/constants.js" as UI

Item {
    id: root

    signal clicked
    signal pressAndHold
    property alias pressed: mouseArea.pressed

    property string imgModel : model.img

    property int titleSize: UI.LIST_TILE_SIZE
    property int titleWeight: Font.Bold
    property color titleColor: theme.inverted ? UI.LIST_TITLE_COLOR_INVERTED : UI.LIST_TITLE_COLOR

    property int bodySize: UI.LIST_SUBTILE_SIZE
    property int bodyWeight: Font.Light
    property color bodyColor: theme.inverted ? UI.LIST_SUBTITLE_COLOR_INVERTED : UI.LIST_SUBTITLE_COLOR

    height: column.height + 4 * UI.MARGIN_XLARGE
    width: parent.width
    scale: mouseArea.pressed? 0.98 : 1.00


    BorderImage {
        id: background
        anchors { fill: parent; topMargin: UI.MARGIN_XLARGE / 2; bottomMargin: UI.MARGIN_XLARGE / 2 }

        source: "image://theme/meegotouch-list"+(theme.inverted?"-inverted":"")+"-background"
        border { left: 21; right: 21; top: 21; bottom: 21 }
    }


    MouseArea {
        id: mouseArea;
        anchors.fill: parent
        onClicked: console.log(">>"+model.stat+ "<<")//root.clicked()
        onPressAndHold:  root.pressAndHold()
    }

    Column {
        id: column

        anchors { top: parent.top; left: parent.left; right: parent.right;
            topMargin: 1.5 * UI.MARGIN_XLARGE }
        height: childrenRect.height
        spacing: UI.MARGIN_XLARGE

        Item {
            anchors { left: parent.left; right: parent.right;
                leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }
            height: poster.height

            Label {
                id: poster
                text: model.poster
                font.family: UI.FONT_FAMILY_BOLD
                font.pixelSize: UI.FONT_DEFAULT_SIZE
                color: theme.inverted ? UI.LIST_SUBTITLE_COLOR_INVERTED : UI.LIST_SUBTITLE_COLOR
            }
            Label {
                id: dateTime
                text: model.dateTime
                anchors.right: parent.right
                font.family: UI.FONT_FAMILY_LIGHT
                font.pixelSize: 18
                color: theme.inverted ? UI.LIST_SUBTITLE_COLOR_INVERTED : UI.LIST_SUBTITLE_COLOR
            }
        }

        Label {
            id: userStat
            text: model.stat
            font.family: UI.FONT_FAMILY_LIGHT
            font.pixelSize: 18
            color: theme.inverted ? UI.LIST_SUBTITLE_COLOR_INVERTED : UI.LIST_SUBTITLE_COLOR
            anchors { left: parent.left; right: parent.right;
                leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }
        }


        Label {
            id: body
            anchors { left: parent.left; right: parent.right;
                leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }

            text: model.body
            font.family: UI.FONT_FAMILY_LIGHT
            font.pixelSize: UI.FONT_LIGHT_SIZE
            onLinkActivated: {
                console.log("Clicked on link:" + link)
                Qt.openUrlExternally(link);
            }
        }

        Repeater {
            id:repItem
            anchors { left: parent.left; right: parent.right; }
            model: if(imgModel != "") imgModel.split("|");
            MouseArea {
                property bool check: true
                anchors { left: parent.left; right: parent.right;
                    leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }
                height: check? UI.LIST_ITEM_HEIGHT : childrenRect.height
                onClicked: check = !check
                onPressAndHold: {
                    Qt.openUrlExternally(modelData);
                }
                clip: true
                Behavior on height {
                    NumberAnimation{}
                }

                BusyIndicator{
                    anchors.centerIn: parent
                    visible: running
                    running: img.status != Image.Ready
                    implicitHeight : 32
                }

                Image{
                    id: img
                    source: modelData
                    sourceSize.width: parent.width
                    asynchronous: true
                    opacity: progress
                    width: parent.width
                    fillMode: Image.PreserveAspectFit
                }
            }
        }



        //Too much memory intensive, although improves smooth scrooling
        //        WebView {
        //            id: body
        //            anchors { left: parent.left; right: parent.right;
        //                      leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }

        //            html: " <html><head>"
        //                  + "</head><body bgcolor=\"#f4f5f5\"><font color=\"#000000\">"
        //                  + model.body
        //                  + "</body></html>"
        //            clip: true
        //            settings.minimumFontSize : 16
        //            settings.defaultFontSize: 22
        //            settings.standardFontFamily: UI.FONT_FAMILY_LIGHT
        //            preferredWidth: width
        //            preferredHeight: 5
        //            settings.javascriptEnabled: false
        //            settings.pluginsEnabled: false
        //        }



        Label {
            id: thanks
            anchors { left: parent.left; right: parent.right;
                leftMargin: UI.MARGIN_XLARGE; rightMargin: UI.MARGIN_XLARGE }

            visible: (model.thanks === undefined) ? false : model.thanks

            horizontalAlignment: Text.AlignRight
            text: "thanks: " + model.thanks
            font.family: UI.FONT_FAMILY_LIGHT
            font.pixelSize: 18
            color: theme.inverted ? UI.LIST_SUBTITLE_COLOR_INVERTED : UI.LIST_SUBTITLE_COLOR
        }
    }
}
