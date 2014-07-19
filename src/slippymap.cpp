#include <QDebug>
#include "slippymap.h"
#include "providers/osmprovider.h"

SlippyMap::SlippyMap(QQuickItem *parent) :
    QQuickPaintedItem(parent),
    currentLocation(gps::Point(53.460477, 6.836115), 10)
{
    setKeepTouchGrab(false);
    addProvider(new OsmProvider(this));
    drawOffset=false;
    setZoom(10);
}

SlippyMap::~SlippyMap()
{
    delete providers[0];
}

void SlippyMap::paint(QPainter *painter)
{
    qDebug()<< "Slippy map paint " << currentLocation;

    QPoint topLeftTilePos(0,0);
    if(drawOffset)
            topLeftTilePos=QPoint (-currentLocation.OffsetPixels().x(), -currentLocation.OffsetPixels().y());

    int hTileCount=(width() /256) + 2;
    int vTileCount=(height()/256) + 2;

    for(int i=0; i<hTileCount; i++)
    for(int j=0; j<vTileCount; j++)
    {
        SlippyCoordinates curTile(currentLocation.zoom(), currentLocation.x()+i, currentLocation.y()+j);
        QPoint tilePos(topLeftTilePos+QPoint(i*256,j*256));
        Tile* tile=providers[0]->getTile(curTile);
        painter->drawImage(tilePos, tile->image);
    }
    for(int i=-1; i<vTileCount+1; i++){
        providers[0]->getTile({currentLocation.zoom(), currentLocation.x()-1, currentLocation.y()+i});
        providers[0]->getTile({currentLocation.zoom(), currentLocation.x()+hTileCount+1, currentLocation.y()+i});
    }
    for(int i=0; i<hTileCount; i++){
        providers[0]->getTile({currentLocation.zoom(), currentLocation.x()+i, currentLocation.y()-1});
        providers[0]->getTile({currentLocation.zoom(), currentLocation.x()+i, currentLocation.y()+vTileCount+1});
    }
}



void SlippyMap::addProvider(SlippyProvider *provider){
    connect(provider, SIGNAL(tileChanged())
            , this, SLOT(onTileChanged())
            );
    providers.append(new SlippyCache(provider));
}

void SlippyMap::makeCache(SlippyProvider *provider){
    //TODO
}

/*
//*/

void SlippyMap::setCoordinates(SlippyCoordinates newCoords)
{
    currentLocation=newCoords;
    update();

}

SlippyCoordinates SlippyMap::coordinates()
{
    return currentLocation;

}

void SlippyMap::onTileChanged()
{
    emit tileChanged();
}

void SlippyMap::reTile(int dx, int dy)
{
    currentLocation.settilePos(currentLocation.tilePos()+QPoint(dx, dy));

   if(dy || dx)
     update();
}

void SlippyMap::setZoom(qreal zoom){
    if(zoom<1)
        return;
    qDebug() << "ZOOMING" << zoom;

    //this is the intpart only, this is the map-layer
    int izoom=zoom;
    if(izoom!=currentLocation.zoom()){
            currentLocation.setzoom(zoom);
        emit tileChanged();
    }

    m_zoom=zoom;

    double buf;
        double floatpart=modf(zoom, &buf);

    setScale(pow(2.0, floatpart));
    emit zoomChanged(zoom);
    update();
}


void SlippyMap::componentComplete()
{
    qDebug()<<"Map is qml component";
    connect(
                this, SIGNAL(tileChanged()),
                this, SLOT(update())
                );
    drawOffset=true;
}
