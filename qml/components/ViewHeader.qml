import QtQuick 1.1
import com.nokia.meego 1.0
import "UIConstants.js" as UI

Item {
    id: root

    property alias text: titleLabel.text
    property alias imageSource: titleImage.source
    property alias showImage: titleImage.visible
    property alias color: background.color

    property Style platformStyle: ViewHeaderStyle {}

    // FIXME - should probably bind to page margins like the scroll decorator does
    anchors { topMargin: -UI.DEFAULT_MARGIN; leftMargin: -UI.DEFAULT_MARGIN; rightMargin: -UI.DEFAULT_MARGIN }

    anchors { top: parent.top; left: parent.left; right: parent.right }
    height: (screen.currentOrientation == Screen.Portrait || screen.currentOrientation == Screen.InvertedPortrait)
            ? UI.HEADER_DEFAULT_HEIGHT_PORTRAIT
            : UI.HEADER_DEFAULT_HEIGHT_LANDSCAPE

    Rectangle {
        id: background
        anchors.fill: parent
        color: platformStyle.backgroundColor

        Rectangle { width: parent.width; height: 2; anchors.bottom: parent.bottom; color: "#000000"; opacity: 0.5 }
        Rectangle { width: parent.width; height: 1; anchors.bottom: parent.bottom; color: "#ffffff" }
    }

    Image {
        id: titleImage
        anchors { left: parent.left; leftMargin: UI.DEFAULT_MARGIN; verticalCenter: parent.verticalCenter }
        source: platformStyle.titleImage
        visible: platformStyle.titleImage
    }

    Label {
        id: titleLabel
        anchors {
            baseline: parent.bottom
            baselineOffset: (screen.currentOrientation == Screen.Portrait ||
                             screen.currentOrientation == Screen.InvertedPortrait)
                            ? -UI.HEADER_DEFAULT_BOTTOM_SPACING_PORTRAIT
                            : -UI.HEADER_DEFAULT_BOTTOM_SPACING_LANDSCAPE
            left: titleImage.visible ? titleImage.right : parent.left
            right: parent.right
            leftMargin: UI.DEFAULT_MARGIN
            rightMargin: UI.DEFAULT_MARGIN
        }
        height: UI.FONT_XLARGE

        font.family: root.platformStyle.fontFamily
        font.pixelSize: root.platformStyle.fontPixelSize
        color: root.platformStyle.textColor
        elide: Text.ElideRight
    }
}
