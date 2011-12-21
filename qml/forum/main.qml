import QtQuick 1.1
import com.nokia.meego 1.0
import Forum 1.0

PageStackWindow {
    id: appWindow

    property Style forumStyle

    onOrientationChangeFinished: {
        showStatusBar = screen.currentOrientation == Screen.Portrait ||
                        screen.currentOrientation == Screen.InvertedPortrait
    }

    initialPage: MainPage { }

    Component.onCompleted: loadForumStyle()

    ForumSession {
        id: forumSession
        provider: "fmc"
        url: "http://forum.meego.com"

        onError: {
            errorDialog.message = message
            errorDialog.open();
        }
        onLoginFailed: {
            errorDialog.message = message
            errorDialog.open()
        }
        onProviderChanged: loadForumStyle()
    }

    QueryDialog {
        id: errorDialog
        titleText: "Session error"
        icon: "image://theme/icon-l-error"
        acceptButtonText: "Ok"
    }

    function loadForumStyle() {
        var style = Qt.createComponent("../forum-" + forumSession.provider + "/ForumStyle.qml")
        forumStyle = style.createObject(appWindow)
    }
}
