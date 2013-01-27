import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1
import Forum 1.0

PageStackWindow {
    id: appWindow

    property Style forumStyle
    property bool dispAvatar : avatarSetting.value
    property int fontSize : fontSetting.value

    onOrientationChangeFinished: {
        showStatusBar = screen.currentOrientation == Screen.Portrait ||
                        screen.currentOrientation == Screen.InvertedPortrait
    }

    initialPage: MainPage { }

    Component.onCompleted: {
        theme.inverted = themeSetting.value
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
    InfoBanner{
        id : infoBanner
        topMargin : 50
        text: "Opening browser..."
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

    GConfItem {
       id: fontSetting
       key: "/apps/forum-qml/settings/fontSetting"
       defaultValue: 0
    }

    function loadForumStyle() {
        var style = Qt.createComponent(forumSession.provider ? ("../forum-" + forumSession.provider + "/ForumStyle.qml") : "ForumStyle.qml")
        forumStyle = style.createObject(appWindow)
    }
}
