#include <QDebug>
#include "slippymap.h"
#include "providers/osmprovider.h"

SlippyMap::SlippyMap(QQuickItem *parent) :
    QQuickPaintedItem(parent),
    currentLocation(gps::Point(53.460477, 6.836115), 10)
{
    addProvider(new OsmProvider(this));
    connect(this, SIGNAL(xChanged()),
            this, SLOT(reTile())
          );
    setAcceptedMouseButtons(Qt::AllButtons);
}

void SlippyMap::paint(QPainter *painter)
{
    qDebug()<< "Slippy map paint";

    QPoint topLeftTilePos(-currentLocation.xOffsetPixels(), -currentLocation.yOffsetPixels());

    int hTileCount=(width() /256) + 2;
    int vTileCount=(height()/256) + 2;

    for(int i=0; i<hTileCount; i++)
    for(int j=0; j<vTileCount; j++)
    {
        SlippyCoordinates curTile(currentLocation.zoom, currentLocation.x+i, currentLocation.y+j);
        QPoint tilePos(topLeftTilePos+QPoint(i*256,j*256));
        Tile* tile=providers[0]->getTile(curTile);
        painter->drawImage(tilePos, tile->image);
    }
    for(int i=-1; i<vTileCount+1; i++){
        providers[0]->getTile({currentLocation.zoom, currentLocation.x-1, currentLocation.y+i});
        providers[0]->getTile({currentLocation.zoom, currentLocation.x+hTileCount+1, currentLocation.y+i});
    }
    for(int i=0; i<hTileCount; i++){
        providers[0]->getTile({currentLocation.zoom, currentLocation.x+i, currentLocation.y-1});
        providers[0]->getTile({currentLocation.zoom, currentLocation.x+i, currentLocation.y+vTileCount+1});
    }


}



void SlippyMap::addProvider(SlippyProvider *provider){
    connect(provider, SIGNAL(tileChanged())
            , this, SLOT(update())
            );
    providers.append(new SlippyCache(provider));
}

void SlippyMap::makeCache(SlippyProvider *provider){
    //TODO
}

void SlippyMap::mousePressEvent(QMouseEvent *event)
{
        dragging=true;
        dragStart=event->pos();
        dragStartCoordinates=currentLocation;
}

void SlippyMap::mouseReleaseEvent(QMouseEvent *event)
{
        dragging=false;
}

void SlippyMap::mouseMoveEvent(QMouseEvent *event)
{
    currentLocation=dragStartCoordinates+ (event->pos()-dragStart);
    update();


}

void SlippyMap::touchEvent(QTouchEvent *event)
{

    QTouchEvent::TouchPoint point = event->touchPoints()[0];
    if(point.state()==Qt::TouchPointMoved){
        QPointF offset(point.pos()-point.lastPos());

        currentLocation=currentLocation+QPoint(-offset.x(), -offset.y());
    }

    if(event->touchPoints().size()==2 && event->touchPointStates()&Qt::TouchPointMoved){

        QTouchEvent::TouchPoint point2=event->touchPoints()[1];



        // vector from finger one to finger two
        QVector2D vector   (point2.pos()-point.pos());
        QVector2D oldVector(point2.lastPos()-point.lastPos());

        qDebug() << "PINCHING";
        qDebug() << oldVector;
        qDebug() << point.id() << ' ' << point2.id();
        qDebug() << point2.lastPos();
        qDebug() << point2.pos();
        qDebug() << point.lastPos();
        qDebug() << point.pos();
        qDebug() << vector;

        //for now, just use the ratio to zoom
        double lengthRatio=vector.length()/oldVector.length();


        setZoom(zoom()*lengthRatio);
        //todo zoom


        //todo rotate
    }


    update();

}

void SlippyMap::setCoordinates(SlippyCoordinates newCoords)
{
    currentLocation=newCoords;
    update();

}

void SlippyMap::reTile(int dx, int dy)
{
   currentLocation.x+=dx;
   currentLocation.y+=dy;

   if(dy || dx)
     update();
}

void SlippyMap::setZoom(qreal zoom){
    if(zoom<1)
        return;
    qDebug() << "ZOOMING" << zoom;

    //this is the intpart only, this is the map-layer
    currentLocation.zoom=zoom;
    m_zoom=zoom;

    double buf;
    double floatpart=modf(zoom, &buf);

    setScale(pow(2.0, floatpart));
    emit zoomChanged(zoom);
    update();
}
