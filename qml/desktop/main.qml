
import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Window 2.0
import Slippy 1.0

Window {
    id: window

    Component.onCompleted: {
        console.log("hello world")
        window.show();

    }

    Row{
        width: parent.width
        height: 30;
        Button{
            id: osmButton;
            text: "osm"
            width: parent.width/2
            onClicked: {
                manager.selectLayer(0);
            }
        }
        Button{
            id: gmButton;
            text: "gmaps"
            width: parent.width/2
            onClicked: {
                manager.selectLayer(1);
            }
        }
    }


    TileManager{
        id: manager
        OsmProvider{

        }
        GoogleMapsProvider{

        }
    }

    SlippyView{
        id:mapview
        anchors.centerIn: parent
        width: parent.width/2;
        height: parent.height/2;
        tileManager: manager
        z: 5

    }
    Rectangle{
        anchors.centerIn: parent
        width: parent.width/2;
        height: parent.height/2;
        color: "#00000000"
        border.color: "black"
        clip: false;
        z: 6
    }
}

