import QtQuick 1.1
import com.nokia.meego 1.0
import "../components"
import "../components/UIConstants.js" as UI
import "../components/constants.js" as UI2

Item {
    id: root

    property alias text: titleLabel.text
    property alias subtitle: subtitleLabel.text

    property Style platformStyle: ViewHeaderStyle {
        fontFamily: UI.FONT_FAMILY
        fontPixelSize: UI.FONT_LARGE
        textColor: "#000"
    }

    // FIXME - should probably bind to page margins like the scroll decorator does
    anchors { topMargin: -UI.DEFAULT_MARGIN; leftMargin: -UI.DEFAULT_MARGIN; rightMargin: -UI.DEFAULT_MARGIN }

    anchors { top: parent.top; left: parent.left; right: parent.right }
    height: titleLabel.height + subtitleLabel.height + 2 * UI.DEFAULT_MARGIN

    Label {
        id: titleLabel
        anchors { left: parent.left; right: parent.right }
        anchors.margins: UI.DEFAULT_MARGIN
        anchors.top: parent.top

        font.family: root.platformStyle.fontFamily
        font.pixelSize: root.platformStyle.fontPixelSize
        color: root.platformStyle.textColor
        elide: Text.ElideRight
        maximumLineCount: 2
    }

    Label {
        id: subtitleLabel
        anchors { left: parent.left; right: parent.right }
        anchors.margins: UI.DEFAULT_MARGIN
        anchors.topMargin: UI.DEFAULT_MARGIN/2
        anchors.top: titleLabel.bottom

        font.family: root.platformStyle.fontFamily
        font.pixelSize: UI.FONT_LSMALL
        color: UI2.LIST_SUBTITLE_COLOR
        elide: Text.ElideRight
    }

    /*
    Image {
        id: separatorImage
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        source: "image://theme/meegotouch-separator-background-horizontal"
    }
    */
}
