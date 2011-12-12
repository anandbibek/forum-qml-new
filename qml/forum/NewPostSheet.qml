import QtQuick 1.1
import com.nokia.meego 1.0
import "UIConstants.js" as UI

Sheet {
    id: root
    rejectButtonText: "Cancel"
 // acceptButtonText: "Post" // This one is blue

    property QtObject forum: null
    property QtObject thread: null
    property QtObject post: null
    property QtObject newPost: null

    Connections {
        target: newPost
        onPreviewChanged: {
            model.body = "<html>" + newPost.preview + "</html>"
            previewDelegate.opacity = 1
        }
        onErrorMessage: {
            errorDialog.message = message
            errorDialog.open()
        }
    }

    Component.onCompleted: {
        if (post) {
            if (thread)
                forumLabel.text = "New reply to thread <span style='color:" + forumStyle.highlightTextColor + "'>" + thread.title + "</span>"
            else
                forumLabel.text = "New reply to <span style='color:" + forumStyle.highlightTextColor + "'>" + post.poster + "</span>"
            topicField.text = "Re: " + post.subject
            var body ="[quote=" + post.poster + (post.postId ? (";" + post.postId + "]") : "]") + post.toBbCode() + "[/quote]"
            bodyArea.text = body
            newPost = forumSession.createNewPost()
            newPost.postId = post.postId
            if (thread)
                newPost.threadId = thread.model.threadId
            post = null
        } else if (thread) {
            forumLabel.text = "New reply to thread <span style='color:" + forumStyle.highlightTextColor + "'>" + thread.title + "</span>"
            topicField.text = "Re: " + thread.title
            newPost = forumSession.createNewPost()
            newPost.threadId = thread.model.threadId
        } else if (forum) {
            forumLabel.text = "New thread in forum <span style='color:" + forumStyle.highlightTextColor + "'>" + forum.title + "</span>"
            newPost = forumSession.createNewPost()
            newPost.forumId = forum.forumId
        }
    }

    TextFieldStyle {
        id: textFieldStyle
        selectionColor: forumStyle.selectionColor
        backgroundSelected: "image://theme/" + forumStyle.colorThemeString + "meegotouch-textedit-background-selected"
    }

    buttons: [
        BusyIndicator {
            anchors.centerIn: parent
            running: visible
            visible: forumSession.busy
        },
        SheetButton {
            id: acceptButton
            objectName: "acceptButton"
            anchors.right: parent.right
            anchors.rightMargin: root.platformStyle.acceptButtonRightMargin
            anchors.verticalCenter: parent.verticalCenter

            // Work around missing color theme support in QML
            platformStyle: SheetButtonAccentStyle {
                background: "image://theme/" + forumStyle.colorThemeString + "meegotouch-sheet-button-accent"+__invertedString+"-background"
                pressedBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-sheet-button-accent"+__invertedString+"-background-pressed"
                disabledBackground: "image://theme/" + forumStyle.colorThemeString + "meegotouch-sheet-button-accent"+__invertedString+"-background-disabled"
            }

            text: postSwitch.checked ? "Post" : "Preview"

            enabled: newPost != null && !forumSession.busy

            onClicked: {
                newPost.subject = topicField.text
                newPost.body = bodyArea.text
                if (postSwitch.checked) {
                    newPost.submit();
                    accept();
                } else {
                    newPost.requestPreview();
                }
            }
        }
    ]

    content: [
        Flickable {
            id: flickable
            anchors.fill: parent
            anchors.margins: UI.DEFAULT_MARGIN
            contentWidth: width
            contentHeight: column.height
            flickableDirection: Flickable.VerticalFlick

            Column {
                id: column
                anchors { left: parent.left; right: parent.right }
                height: childrenRect.height
                spacing: UI.DEFAULT_MARGIN

                Label {
                    id: forumLabel
                    anchors.left: parent.left
                    anchors.right: parent.right
                }

                PostDelegate {
                    id: previewDelegate
                    opacity: 0

                    QtObject {
                        id: model
                        property string poster: "Preview"
                        property string dateTime: "Today"
                        property string body: ""
                    }

                    onClicked: destroy()
                }

                move: Transition {
                    NumberAnimation {
                        properties: "y"
                        easing.type: Easing.OutExpo
                        duration: 150
                    }
                }

                TextField {
                    id: topicField
                    anchors { left: parent.left; right: parent.right }
                    height: 51

                    platformStyle: textFieldStyle
                    placeholderText: "Enter topic here"
                }

                Rectangle {
                    color: "white"
                    anchors { left: parent.left; right: parent.right;
                              leftMargin: -UI.DEFAULT_MARGIN; rightMargin: -UI.DEFAULT_MARGIN }
                    height: bodyArea.height

                    TextArea {
                        id: bodyArea
                        anchors { left: parent.left; right: parent.right }

                        platformStyle: TextFieldStyle {
                            selectionColor: forumStyle.selectionColor

                            background: ""
                            backgroundSelected: ""
                            backgroundDisabled: ""
                            backgroundError: ""
                        }
                        placeholderText: "Enter forum post here"
                        textFormat: Text.PlainText
                    }
                }

                Row {
                    spacing: UI.DEFAULT_MARGIN

                    Switch {
                        id: postSwitch
                        checked: false
                        platformStyle: SwitchStyle {
                            switchOn: "image://theme/" + forumStyle.colorThemeString + "meegotouch-switch-on"+__invertedString
                        }
                    }

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Enable posting"
                    }
                }
            }
        },
        ScrollDecorator {
            flickableItem: flickable
        }
    ]

    onAccepted: newPost.destroy()
    onRejected: newPost.destroy()

    QueryDialog {
        id: errorDialog
        titleText: "Error message received"
        icon: "image://theme/icon-l-error"
        acceptButtonText: "Ok"
    }
}
