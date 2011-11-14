import QtQuick 1.1
import com.nokia.meego 1.0 // for Style
import "UIConstants.js" as UI

Style {

    // Font
    property int firstCharacterFontPixelSize: 70
    property int fullStringFontPixelSize: 36

    // Color
    property color textColor: inverted?"#000":"#fff"

    property string handleImage: "image://theme/meegotouch-fast-scroll-handle"+__invertedString
    property string magnifierImage: "image://theme/meegotouch-fast-scroll-magnifier"+__invertedString
    property string railImage: "image://theme/meegotouch-fast-scroll-rail"+__invertedString
}
