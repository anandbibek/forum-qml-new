import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.extras 1.1
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
                platformSipAttributes: SipAttributes {
                    actionKeyLabel: "Next"
                    actionKeyHighlighted: true
                }
                platformStyle: textFieldStyle

                onTextChanged: if (text) errorHighlight = false

                Keys.onPressed: {
                    if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                        passwordField.focus = true
                    }
                }
            }
        }

        Column {
            width: parent.width

            Label { text: "Password"; platformStyle: labelStyle }
            TextField {
                id: passwordField
                width: parent.width
                echoMode: TextInput.Password
                platformSipAttributes: SipAttributes {
                    actionKeyLabel: "Ready"
                    actionKeyHighlighted: true
                }
                platformStyle: textFieldStyle

                onTextChanged: if (text) errorHighlight = false

                Keys.onPressed: {
                    if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                        platformCloseSoftwareInputPanel()
                        tryLogin()
                    }
                }
            }
        }

        Button {
            id: loginButton
            anchors.horizontalCenter: parent.horizontalCenter
            enabled: (userField.text || passwordField.text) && !forumSession.busy
            text: "Login"
            platformStyle: ButtonStyle {
                pressedBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-button" + __invertedString + "-background-pressed"
            }

            onClicked: tryLogin()
        }
    }

    ToolBarLayout {
        id: settingsTools
        visible: true
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: pageStack.pop()
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: visible
            visible: forumSession.busy
        }
    }

    Connections {
        target: forumSession
        onLoginSucceeded: pageStack.pop()
    }

    // Focus the username field (and pop up VKB) after the page stack animation finished
    onStatusChanged: {
        if (status == PageStatus.Active) {
            userField.focus = true
        }
    }

    function tryLogin() {
        userField.errorHighlight = (!userField.text)
        passwordField.errorHighlight = (!passwordField.text)
        if (userField.text && passwordField.text) {
            forumSession.userName = userField.text
            forumSession.password = passwordField.text
            forumSession.login()
        }
    }
}
