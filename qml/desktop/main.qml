
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
        Button{
            id: osmButton;
            text: "osm"
            width: parent.width/2
            onClicked: {
                mapview.selectLayer(0);
            }
        }
        Button{
            id: gmButton;
            text: "gmaps"
            width: parent.width/2
            onClicked: {
                mapview.selectLayer(1);
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
        tileManager: manager
        anchors.fill: parent
        z: -5

    }
}

