import QtQuick 1.1
import com.nokia.meego 1.0
import "../forum"
import Forum 1.0

PageStackWindow {
    id: appWindow

    onOrientationChangeFinished: {
        showStatusBar = screen.currentOrientation == Screen.Portrait ||
                        screen.currentOrientation == Screen.InvertedPortrait
    }

    initialPage: MainPage { }

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
    }

    ForumStyle {
        id: forumStyle

        viewHeaderTextColor: "#333333"
        viewHeaderBackgroundColor: "#ffffff"

        forumTitleTextColor: "#333333"
        highlightTextColor: "#cc09ba"
        selectionColor: "#cc09ba"

        colorThemeString: "color12-"

        titleImage: "file:///opt/forum-fmc/images/meego.png"
        titleText: "Forum"
    }

    QueryDialog {
        id: errorDialog
        titleText: "Session error"
        icon: "image://theme/icon-l-error"
        acceptButtonText: "Ok"
    }
}
