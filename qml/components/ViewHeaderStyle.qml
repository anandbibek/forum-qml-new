import QtQuick 1.1
import com.nokia.meego 1.0
import "UIConstants.js" as UI

Style {
    // Font
    property string fontFamily: "Nokia Pure Text Light" //UI.FONT_FAMILY
    property int fontPixelSize: UI.FONT_XLARGE

    // Color
    property color textColor: forumStyle.viewHeaderTextColor
    property color backgroundColor: forumStyle.viewHeaderBackgroundColor

    property string fixedImage: "image://theme/meegotouch-view-header-fixed"+__invertedString

    property string titleImage: ""
}
