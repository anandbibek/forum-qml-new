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
    property bool setFromWebPage : true

    opacity: Math.max((1-(Math.abs(x)+Math.abs(y))/(1200)),0.5)

    Connections {
        target: newPost
        onPreviewChanged: {
            var temp = newPost.preview.split("##splitMarker##")
            model.body = "<html>" + temp[0] + "</html>"

            if(temp.length>1)
                model.img = temp[1]

            previewDelegate.opacity = 1
            divider.expanded = true
        }
        onEditTextChanged : {
            if(editPost && setFromWebPage)
                bodyArea.text = newPost.editText
            setFromWebPage = false
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

    Rectangle{
        height: 70
        color: "#ea650a"
        width: 70
        radius: 5
        x : root.x>=0 ? -70 : root.width
        //anchors.right: parent.left
        anchors.top: parent.top
        Image{
            asynchronous: true
            anchors.centerIn: parent
            source: "image://theme/icon-m-image-edit-resize"
        }
        MouseArea{
            //anchors.fill: parent
            anchors.fill: parent
            drag.target: root
            drag.axis: Drag.XandYAxis
            drag.maximumX: root.width
            drag.minimumX: -root.width
            drag.maximumY: root.height-70
            drag.minimumY: 0
            onClicked: root.z++
        }
    }

    buttons: [
        MouseArea{
            anchors.fill: parent
            drag.target: root
            drag.axis: Drag.XandYAxis
            drag.maximumX: root.width-48
            drag.minimumX: 48-root.width
            drag.maximumY: root.height-70
            drag.minimumY: 0
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
            Image{
                asynchronous: true
                anchors.centerIn: parent
                visible: !forumSession.busy
                source: "image://theme/icon-m-image-edit-resize"
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

                text: (actionPanel.selectedIndex == 1) ? (editPost? "Save" : "Post") : "Preview"

                enabled: newPost != null && !forumSession.busy

                onClicked: {
                    newPost.subject = topicField.text
                    if(bodyArea.text.length<10)
                        bodyArea.text += "\n-via Meamo Talk app"
                    newPost.body = bodyArea.text
                    if (actionPanel.selectedIndex == 1) {
                        if(editPost)
                            newPost.edit(false);
                        else
                            newPost.submit();
                        accept();
                    } else {
                        if(editPost)
                            newPost.edit(true)
                        else
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
                        property string img
                    }

                    onClicked: destroy()
                }

                ComboPanel {
                    id: actionPanel
                    title: "Action: "
                    visible: divider.expanded
                    selectedIndex: 1
                    model: ListModel {
                        ListElement { name: "Preview" }
                        ListElement { name: "Post" }
                    }
                }
                Label {
                    id: toolbarLabel
                    height: 56
                    text: "Tools: "
                    verticalAlignment: Text.AlignVCenter
                    anchors { left: parent.left; right: parent.right;}
                    visible: divider.expanded
                    Label {
                        text: "\tOpen"
                        anchors.verticalCenter: parent.verticalCenter
                        font.weight: Font.Bold
                    }
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            loader.sourceComponent = toolbar
                        }
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
                        anchors { top: parent.top; left: parent.left; right: parent.right; topMargin: -1 }
                        height: Math.max(parent.height, parent.height + flickable.height - column.height + 16)
                        color: "white"
                        z: -1
                    }

                    MouseArea{
                        anchors.fill: parent
                        visible: (forumSession.busy && editPost && setFromWebPage)
                        Rectangle{
                            color: "black"
                            anchors.fill: parent
                            anchors.topMargin: -48
                            opacity: 0.75
                        }
                        BusyIndicator{
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: 100
                            running: true
                            implicitWidth: 96
                        }
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
        y:toolbarLabel.y
        width: 0
    }

    Component{
        id : toolbar

        ButtonRow{
            id : row
            exclusive: false
            opacity: 0
            Component.onCompleted: opacity = 1
            Behavior on opacity {NumberAnimation{}}

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
            if(!editPost)
                bodyArea.forceActiveFocus();
            column.move = columnTransition
        }
    }
}
