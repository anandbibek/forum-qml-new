import QtQuick 1.1
import com.nokia.meego 1.1
import "../forum/UIConstants.js" as UI

MouseArea{


    property QtObject thread : null


    id : customDialog

    anchors{
        horizontalCenter: parent.horizontalCenter
        verticalCenter: parent.verticalCenter
    }
    width: parent.width
    opacity: 0
    clip: true

    onClicked: {
        height = 0
        width = 0
        opacity = 0
        customDialog.destroy(100)
    }


    Component.onCompleted: {
        height = parent.height + 100
        opacity = 1
        slider.maximumValue = thread.model.numPages
        slider.value = thread.model.lastPage
    }
    Behavior on opacity{NumberAnimation{duration: 300}}

    Rectangle{
        color: "#000000"
        opacity: 0.90
        anchors.fill: parent
    }

    Column{
        spacing: 24
        width: parent.width
        anchors{
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        Slider{
            id : slider
            stepSize: 1
            minimumValue: 1
            valueIndicatorVisible: true
            anchors{
                left: parent.left
                right: parent.right
            }
        }
        Row{
            width: parent.width
            spacing: 12

            Button{
                text: "-"
                width: 51
                onClicked: {
                    slider.value--
                }
            }
            Button{
                text: "Jump to page " + slider.value
                checked: true
                width: parent.width - 126
                onClicked: {
                    thread.model.load(slider.value)
                    customDialog.destroy(100)
                }
            }
            Button{
                text: "+"
                width: 51
                onClicked: {
                    slider.value++
                }
            }
        }


    }
}
