import QtQuick 1.1
import com.nokia.meego 1.1
import Forum 1.0

PageStackWindow {
    id: appWindow

    property Style forumStyle
    property bool dispAvatar

    onOrientationChangeFinished: {
        showStatusBar = screen.currentOrientation == Screen.Portrait ||
                        screen.currentOrientation == Screen.InvertedPortrait
    }

    initialPage: MainPage { }

    Component.onCompleted: {
        theme.inverted = themeSetting.value
        dispAvatar = avatarSetting.value
        loadForumStyle()
        if (!forumSession.provider)
            Qt.createComponent("ForumSelectionDialog.qml").createObject(initialPage, {"selectedIndex": 0}).open()
    }

    Connections {
        target: forumSession
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

    GConfItem {
       id: themeSetting
       key: "/apps/forum-qml/settings/themeSetting"
       defaultValue: false
    }

    GConfItem {
       id: avatarSetting
       key: "/apps/forum-qml/settings/avatarSetting"
       defaultValue: true
    }

    function loadForumStyle() {
        var style = Qt.createComponent(forumSession.provider ? ("../forum-" + forumSession.provider + "/ForumStyle.qml") : "ForumStyle.qml")
        forumStyle = style.createObject(appWindow)
    }
}
