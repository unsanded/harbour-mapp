/*
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import Slippy 1.0
import QtPositioning 5.0

Page {
    id: root

    TileManager{
        id: manager
        OsmProvider{

        }
        GoogleMapsProvider{

        }
    }
    //proberpty bool dualView: dualCheck.checked

        PageHeader{
            id:header
            title: "MapView testarea"
        }

        Column{
            anchors.top: header.bottom
            anchors.bottom: parent.bottom
            width: parent.width
            spacing: Theme.paddingLarge

            Row{
                id: controls1
                width: parent.width
                Switch{
                    id:controlsVisible
                }

                Button{
                    text: "Osm"
                    color: "red"
                    onClicked: {
                        manager.selectLayer(1)
                    }
                }
                Button{
                    text: "gmap"
                    color: "red"

                    onClicked: {
                        manager.selectLayer(0)
                    }
                }
            }
            Row{
                visible: controlsVisible.checked
                width: parent.width
                Slider{
                    width:parent.width
                    id: zoomSlider
                    value: 4
                    minimumValue: 4
                    maximumValue: 18
                    stepSize: 0.1
                }
            }
            Row{
                visible: controlsVisible.checked
                width: parent.width
                Slider{
                    width:parent.width - gpsButton.width
                    id: rotationSlider
                    maximumValue: 6
                    stepSize: 0.1
                }
                IconButton {
                    id: gpsButton
                    width: icon.width
                    height: parent.height
                    icon.source: "image://theme/icon-m-gps"


                    onClicked: {
                        positionSource.start();
                        console.log("Started Gps")
                    }
                }
            }
        }




        SlippyView{
            tileManager: manager
            anchors.fill: parent
            z: -5
            id:mapview
            zoom: zoomSlider.value
            mapRotation: rotationSlider.value

            lockRotation: true;
            lockZoom:  false;
        }


        //stole this bit from fahrpan
    PositionSource {
        id: positionSource
        active: false

        onPositionChanged: {

            //Only do something if active
            if (positionSource.active === false) {
                return;
            }

            if (positionSource.position.latitudeValid && positionSource.position.longitudeValid) {
                console.log(qsTr("found position..."));
                mapview.setlocation(positionSource.position.coordinate)
                console.log("timestamp" + positionSource.position.timestamp)
                console.log("set Location")

            } else {
                console.log(qsTr("Waiting for GPS lock..."));
            }
        }
    }
}





