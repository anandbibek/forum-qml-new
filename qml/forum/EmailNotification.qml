// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.1
import Forum 1.0

SelectionDialog {
    id: folderDialog

    property QtObject thread: null

    titleText: "Select Email Notification"

    model: ListModel {
        ListElement { name: "No email notification" }
        ListElement { name: "Instant notification by email" }
        ListElement { name: "Daily updates by email" }
        ListElement { name: "Weekly updates by email" }
    }
    onSelectedIndexChanged: {
        if(model){
        forumSession.setNotif(selectedIndex);
        forumSession.subscribe(thread)
        }
    }
    Component.onCompleted: selectedIndex = (forumSession.notif)*1;
}
