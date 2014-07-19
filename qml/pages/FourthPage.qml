import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height


        ListModel {

            id:model;
            ListElement {
                name: "Jim Williams"
            }
            ListElement {
                name: "John Brown"
            }
            ListElement {
                name: "Bill Smyth"
            }
            ListElement {
                name: "Sam Wise"
            }
        }
        GridView{

            cellWidth: 256;
            cellHeight: 256;
            anchors.fill: parent
            model:model
            delegate: Rectangle{
                color: "#00ff3b"
                border.color: "black"
                width: 256
                height: 256
                Label{
                    text: name;
                }
            }
        }
    }
}
