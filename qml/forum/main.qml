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

        legal: 'This program is free software: you can redistribute it and/or modify ' +
            'it under the terms of the GNU General Public License as published by ' +
            'the Free Software Foundation, either version 3 of the License, or ' +
            '(at your option) any later version.<br /><br />' +

            'This package is distributed in the hope that it will be useful, ' +
            'but WITHOUT ANY WARRANTY; without even the implied warranty of ' +
            'MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ' +
            'GNU General Public License for more details.<br /><br />' +

            'You should have received a copy of the GNU General Public License ' +
            'along with this program. If not, see ' +
            '<a href="http://www.gnu.org/licenses" style="color:#cc09ba">http://www.gnu.org/licenses</a><br /><br />' +

            'MeeGo and all related marks and logos are registered trademarks of The Linux Foundation. ' +
            'This application is not, in any way, endorsed by the ' +
            '<a href="http://meego.com" style="color:#cc09ba">MeeGo project</a> or ' +
            '<a href="http://linuxfoundation.org" style="color:#cc09ba">The&nbsp;Linux&nbsp;Foundation</a>.<br /><br />' +

            'The source code of this program can be downloaded from Gitorious: ' +
            '<a href="https://gitorious.org/forum-qml/forum-fmc" style="color:#cc09ba">https://gitorious.org/forum-qml/forum-fmc</a><br /><br />'
    }

    QueryDialog {
        id: errorDialog
        titleText: "Session error"
        icon: "image://theme/icon-l-error"
        acceptButtonText: "Ok"
    }
}
