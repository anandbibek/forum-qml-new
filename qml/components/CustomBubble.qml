import QtQuick 1.1
import com.nokia.meego 1.1
import "UIConstants.js" as C

/*
   Class: CountBubble
   CountBubble component is a flexible shape that holds a number and can be added in lists or
   notification banners for example.
*/

Item {
    id: root

    /*
     * Property: largeSized
     * [bool=false] Use small or large count bubble.
     */
    property bool largeSized: false

    /*
     * Property: value
     * [int=0] Reflects the current value.
     */
    property int value: 0

    implicitWidth: internal.getBubbleWidth()
    implicitHeight: largeSized ? 32:24

    BorderImage {
        source: "image://theme/meegotouch-countbubble-background"+(largeSized ? "-large":"")
        anchors.fill: parent
        border { left: 10; top: 10; right: 10; bottom: 10 }
    }

    Label {
        id: text
        height: parent.height
        y:1
        opacity: 0.75
        font.family: C.FONT_FAMILY
        anchors.horizontalCenter: parent.horizontalCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: largeSized ? 22:18
        text: root.value
    }

    QtObject {
        id: internal

        function getBubbleWidth() {
            if (largeSized) {
                if (root.value < 10)
                    return 32;
                else if (root.value < 100)
                    return 40;
                else if (root.value < 1000)
                    return 52;
                else
                    return text.paintedWidth+19
            } else {
                if (root.value < 10)
                    return 24;
                else if (root.value < 100)
                    return 30;
                else if (root.value < 1000)
                    return 40;
                else
                    return text.paintedWidth+13
            }
        }
    }
}
