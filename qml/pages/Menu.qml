import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQuick.Window 2.0
import Slippy 1.0

import "../pages"


Page {
    PageHeader{
        Label{
            text: "choose"
        }
    }
    SilicaFlickable{
        Button{
            text:"P1";
            id: p1
            onClicked: {
                pageStack.push(Qt.resolvedUrl("FirstPage.qml"));
            }
            anchors.top: parent.top
        }
        Button{
            text:"P2"
            id: p2
            onClicked: {
                pageStack.push(Qt.resolvedUrl("SecondPage.qml"));
            }
            anchors.top:p1.bottom
        }
        Button{
            text:"P3";
            id: p3
            onClicked: {
                pageStack.push(Qt.resolvedUrl("ThirdPage.qml"));
            }
            anchors.top:p2.bottom
        }
        Button{
            text:"P4";
            onClicked: {
                pageStack.push(Qt.resolvedUrl("FourthPage.qml"));
            }
            anchors.top:p3.bottom
        }
    }
}
