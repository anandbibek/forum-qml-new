import QtQuick 1.1
import com.nokia.meego 1.1
import "../components"
import "UIConstants.js" as UI

Sheet {
    id: root
    // rejectButtonText: "Cancel"
    // acceptButtonText: "Post" // This one is blue

    property QtObject forum: null
    property QtObject thread: null
    property QtObject post: null
    property QtObject newPost: null
    property bool editPost: false

    Connections {
        target: newPost
        onPreviewChanged: {
            model.body = "<html>" + newPost.preview + "</html>"
            previewDelegate.opacity = 1
            divider.expanded = true
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
            var body
            if(editPost)
                body = post.toBbCode()
            else
                body ="[quote=" + post.poster + (post.postId ? (";" + post.postId + "]") : "]") + post.toBbCode() + "[/quote]"
            bodyArea.text = body
            newPost = forumSession.createNewPost()
            newPost.setEditPost(editPost)
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

    buttons: [
        MouseArea{
            //anchors.fill: parent
            height: parent.height
            width: parent.width
            drag.target: root
            drag.axis: Drag.XAxis
            drag.maximumX: root.width-48
            onClicked: root.z++

            SheetButton {
                id: rejectButton
                objectName: "rejectButton"
                anchors.left: parent.left
                anchors.leftMargin: root.platformStyle.rejectButtonLeftMargin
                anchors.verticalCenter: parent.verticalCenter
                text: "Cancel"
                onClicked: reject()
            }
            BusyIndicator {
                anchors.centerIn: parent
                running: visible
                visible: forumSession.busy
            }
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

                text: (actionPanel.selectedIndex == 1) ? "Post" : "Preview"

                enabled: newPost != null && !forumSession.busy

                onClicked: {
                    newPost.subject = topicField.text
                    newPost.body = bodyArea.text
                    if (actionPanel.selectedIndex == 1) {
                        if(editPost)
                            newPost.edit();
                        else
                            newPost.submit();
                        accept();
                    } else {
                        newPost.requestPreview();
                    }
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

                Label {
                    id: forumLabel
                    anchors { left: parent.left; right: parent.right }
                    font.pixelSize: UI.FONT_LSMALL
                    font.family: "Nokia Pure Text Light"
                }

                PostDelegate {
                    id: previewDelegate
                    opacity: 0
                    visible: divider.expanded

                    QtObject {
                        id: model
                        property string poster: "Preview"
                        property string dateTime: "Today"
                        property string body: ""
                    }

                    onClicked: destroy()
                }

                ComboPanel {
                    id: actionPanel
                    title: "Action:  "
                    visible: divider.expanded
                    selectedIndex: 1
                    model: ListModel {
                        ListElement { name: "Preview" }
                        ListElement { name: "Post" }
                    }
                }
                Label {
                    id: toolbarLabel
                    text: "Tools:  <B>Open</B>"
                    anchors { left: parent.left; right: parent.right;}
                    visible: divider.expanded
                    MouseArea{
                        anchors.fill: parent
                        onClicked: loader.sourceComponent = toolbar
                    }
                }

                TextPanel {
                    id: topicField
                    selectionColor: forumStyle.selectionColor
                    title: "Topic:"
                    placeholderText: "Enter topic here"
                }

                RecipientDivider {
                    id: divider
                    anchors { leftMargin: -16; rightMargin: -16 }
                }

                TextArea {
                    id: bodyArea
                    anchors { left: parent.left; right: parent.right;
                        leftMargin: -UI.DEFAULT_MARGIN ; rightMargin: -UI.DEFAULT_MARGIN }

                    platformStyle: TextFieldStyle {
                        selectionColor: forumStyle.selectionColor

                        background: ""
                        backgroundSelected: ""
                        backgroundDisabled: ""
                        backgroundError: ""
                    }
                    placeholderText: "Enter forum post here"
                    textFormat: Text.PlainText

                    Rectangle {
                        anchors { top: parent.top; left: parent.left; right: parent.right }
                        height: Math.max(parent.height, parent.height + flickable.height - column.height + 16)
                        color: "white"
                        z: -1
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

    Loader{
        id: loader
        x:24
        y:48
    }

    Component{
        id : toolbar

        ButtonRow{
            id : row
            exclusive: false
            Button{
                iconSource: "image://theme/icon-m-toolbar-stop"+(theme.inverted?"-white":"")+""

                MouseArea{
                    anchors.fill: parent
                    drag.target: row
                    onClicked: loader.sourceComponent = undefined
                    drag.filterChildren: true
                }
            }
            Button{
                text: "B"
                onClicked: bodyArea.text +="[B][/B]"
            }
            Button{
                text: "I"
                onClicked: bodyArea.text +="[I][/I]"
            }
            Button{
                text: "U"
                onClicked: bodyArea.text +="[U][/U]"
            }
            Button{
                text: "IMG"
                onClicked: bodyArea.text +="[IMG][/IMG]"
            }
            Button{
                text: "\" \""
                onClicked: bodyArea.text +="[QUOTE][/QUOTE]"
            }
            Button{
                text: "< >"
                onClicked: bodyArea.text +="[CODE][/CODE]"
            }
        }
    }

    Transition {
        id: columnTransition
        NumberAnimation {
            properties: "y"
            duration: 600
            easing.type: Easing.OutBack
        }
    }

    onStatusChanged: {
        if (status == DialogStatus.Open) {
            bodyArea.forceActiveFocus();
            column.move = columnTransition
        }
    }
}
