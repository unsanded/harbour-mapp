
import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQuick.Window 2.0
import Slippy 1.0

import "../pages"


Page {
        PageHeader{
            Text {
                id: title
                text: qsTr("mapView")
            }
        }

    id:page
    Rectangle{
        border.color: "red"
        anchors.top: parent.top
        height: parent.height/2
        color: "#297a12"

        Label{
            id:lb
            anchors.top:parent.top
            text: "sometext"
        }

        SlippyMapGl{

                anchors.top: lb.bottom
                anchors.bottom: parent.bottom
                width:parent.width
        }
    }
}
