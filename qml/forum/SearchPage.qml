import QtQuick 1.1
import com.nokia.meego 1.0
import "../components"
import "UIConstants.js" as UI

Page {
    id: root
    anchors.margins: UI.DEFAULT_MARGIN

    property QtObject threads: null

    ViewHeader {
        id: header
        height: 80 // like the Search application
        z: 1

        TextField {
            id: searchField
            anchors { left: parent.left; right: parent.right }
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: UI.DEFAULT_MARGIN
            height: 50

            platformSipAttributes: SipAttributes {
                actionKeyLabel: "Ready"
                actionKeyHighlighted: true
             // actionKeyEnabled: true
            }

            platformStyle: TextFieldStyle {
                selectionColor: forumStyle.selectionColor
                backgroundSelected: "image://theme/" + forumStyle.colorThemeString + "meegotouch-textedit-background-selected"
                paddingRight: searchButton.width
            }

            placeholderText: "Search"
            Image {
                id: searchButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                source: searchField.text ? "image://theme/icon-m-input-clear" : "image://theme/icon-m-common-search"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (searchField.text) {
                            searchField.text = ""
                            if (threads) {
                                threads.destroy()
                                threads = null
                            }
                        }
                    }
                }
            }

            Keys.onPressed: {
                if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
                    if (searchField.text != "")
                        threads = forumSession.search(searchField.text)
                    searchField.platformCloseSoftwareInputPanel()
                    threadList.focus = true
                }
            }
        }
    }

    ListView {
        id: threadList

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.topMargin: UI.DEFAULT_MARGIN
        anchors.bottom: parent.bottom
        cacheBuffer: 4000

        section.delegate: SectionHeader { }
        section.property: "section"

        delegate: ThreadDelegate {
            platformStyle: ThreadDelegateStyle { showForum: true }
            onClicked: openThread(threads.get(index))
            onPressAndHold: {
                threadMenu.index = index
                threadMenu.open()
            }
        }
        model: threads

        footer: Button {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: threads != null && threads.count > 0 && threads.lastPage < threads.numPages
            enabled: !forumSession.busy
            text: "Older threads"
            onClicked: threads.load(threads.lastPage + 1)
        }
    }

    FastScroll {
        listView: threadList
        visible: threads != null && threads.count > 0
    }

    Label {
        id: emptyLabel
        anchors.centerIn: parent

        color: UI.COLOR_SECONDARY_FOREGROUND
        font.family: UI.FONT_FAMILY_LIGHT
        font.pixelSize: 64
        visible: threads != null && threads.count == 0 && !forumSession.busy

        text: "No matches"
    }

    BusyIndicator {
        anchors.centerIn: threadList
        platformStyle: BusyIndicatorStyle { size: "large" }
        running: visible
        visible: forumSession.busy && threads != null && threads.count == 0
    }

    tools: ToolBarLayout {
        ToolIcon {
            platformIconId: "toolbar-back"
            onClicked: {
                if (threads)
                    threads.destroy(250)
                pageStack.pop()
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: visible
            visible: forumSession.busy && threads != null && threads.count > 0
        }
    }

    // Focus the search field (and pop up VKB) after the page stack animation finished
    onStatusChanged: {
        if (status == PageStatus.Active) {
            searchField.focus = true
        }
    }

    // FIXME - duplicated from ThreadListView
    function openThread(thread, openMode) {
        if (openMode)
            thread.openMode = openMode

        pageStack.push(Qt.createComponent("ThreadPage.qml"), {"thread": thread});
    }
}
