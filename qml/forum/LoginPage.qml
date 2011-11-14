import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.0
import "UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    tools: settingsTools

    LabelStyle {
        id: labelStyle
        fontFamily: UI.FONT_FAMILY_LIGHT
        fontPixelSize: UI.FONT_LSMALL
    }

    TextFieldStyle {
        id: textFieldStyle
        selectionColor: forumStyle.selectionColor
        backgroundSelected: "image://theme/" + forumStyle.colorThemeString + "meegotouch-textedit-background-selected"
    }

    Column {
        anchors.fill: parent
        spacing: UI.DEFAULT_MARGIN

        Label {
            width: parent.width
            text: "Login to be able to post messages."
        }

        Image {
            width: parent.width
            source: "image://theme/meegotouch-separator-background-horizontal"
        }

        Column {
            width: parent.width

            Label { text: "Username"; platformStyle: labelStyle }
            TextField {
                id: userField
                width: parent.width
                platformStyle: textFieldStyle

                onTextChanged: if (text) errorHighlight = false
            }
        }

        Column {
            width: parent.width

            Label { text: "Password"; platformStyle: labelStyle }
            TextField {
                id: passwordField
                width: parent.width
                echoMode: TextInput.Password
                platformStyle: textFieldStyle

                onTextChanged: if (text) errorHighlight = false
            }
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            enabled: (userField.text || passwordField.text) && !forumSession.busy
            text: "Login"
            platformStyle: ButtonStyle {
                pressedBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-button" + __invertedString + "-background-pressed"
            }

            onClicked: {
                userField.errorHighlight = (!userField.text)
                passwordField.errorHighlight = (!passwordField.text)
                if (userField.text && passwordField.text) {
                    forumSession.userName = userField.text
                    forumSession.password = passwordField.text
                    forumSession.login()
                }
            }
        }
    }

    ToolBarLayout {
        id: settingsTools
        visible: true
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }
    }

    Connections {
        target: forumSession
        onLoginSucceeded: pageStack.pop()
    }
}
