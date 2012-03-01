import QtQuick 1.1
import com.nokia.meego 1.0

Item {
    id: root
    height: 56
    anchors.left: parent.left
    anchors.right: parent.right

    property alias title: label.text
    property alias model: selectionDialog.model
    property alias selectedIndex: selectionDialog.selectedIndex

    Label {
        id: label
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        z:1
        anchors.fill: parent
        anchors.leftMargin: -16
        anchors.rightMargin: -16
        onClicked: selectionDialog.open()
    }

    Label {
        id: valueLabel
        anchors.left: label.right
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        font.weight: Font.Bold
    }

    SelectionDialog {
        id: selectionDialog
        titleText: root.title

        onSelectedIndexChanged: {
            if (model) {
                var selection = model.get(selectedIndex)
                if (selection)
                    valueLabel.text = selection.name
            }
        }
        onModelChanged: {
            var selection = model.get(selectedIndex)
            if (selection)
                valueLabel.text = selection.name
        }
    }
}
