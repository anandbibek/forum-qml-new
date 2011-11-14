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
        provider: "tmo"
        url: "http://talk.maemo.org"

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

        viewHeaderTextColor: "#ffffff"
        viewHeaderBackgroundColor: "#424345"

        forumTitleTextColor: "#ea650a"
        highlightTextColor: "#ea650a"
        selectionColor: "#ea650a"

        colorThemeString: "color15-"

        titleImage: "file:///opt/tmo/images/maemo.org.png"
        titleText: "- Talk"
    }

    QueryDialog {
        id: errorDialog
        titleText: "Session error"
        icon: "image://theme/icon-l-error"
        acceptButtonText: "Ok"
    }
}
