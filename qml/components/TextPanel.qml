import QtQuick 1.1
import com.nokia.meego 1.1
import "UIConstants.js" as UI

Item {
    id: root
    height: 56
    anchors.left: parent.left
    anchors.right: parent.right

    property alias title: label.text
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property color selectionColor

    BorderImage {
        id: background
        anchors { fill: parent; leftMargin: -UI.MARGIN_XLARGE; rightMargin: -UI.MARGIN_XLARGE }
        source: textField.activeFocus ? "image://theme/meegotouch-editor-inputfield-panel"+(theme.inverted?"-inverted":"")+"-background-selected" : ""
        border.left: UI.CORNER_MARGINS
    }

    Label {
        id: label
        anchors.verticalCenter: parent.verticalCenter
        color: root.selectionColor
        opacity: textField.activeFocus ? 1 : 0.5
    }

    MouseArea {
        enabled: !textField.activeFocus
        z: enabled ? 1 : 0
        anchors { fill: parent; leftMargin: -UI.MARGIN_XLARGE; rightMargin: -UI.MARGIN_XLARGE }
        onClicked: {
            if (!textField.activeFocus) {
                textField.forceActiveFocus();
            }
        }
    }

    TextField {
        id: textField
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: label.right
        anchors.right: parent.right
        font.pixelSize: UI.FONT_DEFAULT
        font.weight: text ? Font.Bold : Font.Normal
        platformStyle: TextFieldStyle {
            textColor: UI.COLOR_SECONDARY_FOREGROUND
            background: ""
            backgroundSelected: ""
            backgroundDisabled: ""
            backgroundError: ""
            paddingLeft: UI.MARGIN_XLARGE
        }
    }

    function forceActiveFocus() {
        console.log("FOCUS")
        textField.forceActiveFocus()
    }
}
