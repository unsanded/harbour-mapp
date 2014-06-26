#include "tile.h"
#include <math.h>
#include <QDebug>

Tile::Tile(SlippyCoordinates coordinates, QObject *parent) :
    image(256, 256, QImage::Format_ARGB32),
    QObject(parent),
  coords(coordinates)
{
    //just to test
    QPainter p;
    p.begin(&image);
    p.setPen(Qt::green);

    p.drawArc(25,25,200,200,0,100);
    p.end();

}

//secosine for convenience
inline double sec(double in){
    return 1.0/cos(in);
}

SlippyCoordinates::SlippyCoordinates(gps::Point location, int zoom)
{
    this->zoom=zoom;

    // for the calculation see:
    // http://wiki.openstreetmap.org/wiki/Tilenames#X_and_Y

    double latitude=location.latitude;
    double longitude=location.longitude;

    double numTiles=pow(2,zoom);

    double xTile = numTiles * ((longitude + 180) / 360);

    double yTile = numTiles * (1.0 - (log(tan(latitude/180.0*M_PI) + sec(latitude/180.0*M_PI)) / M_PI)) / 2.0;

    x=xTile;
    y=yTile;

    double buf;
    offsetX=modf(xTile,&buf);
    offsetY=modf(yTile,&buf);
    m_hasOffset=true;


}

bool SlippyCoordinates::hasOffset() const
{
    return m_hasOffset;
}



SlippyCoordinates SlippyCoordinates::operator+(QPoint offset)
{
   QPointF fpoint=offset;
   fpoint/=256.0;
   return operator+(fpoint);
}

SlippyCoordinates SlippyCoordinates::operator+(QPointF offset)
{
    SlippyCoordinates ret;
    ret.zoom=zoom;
    ret.offsetX=offsetX+offset.x();
    ret.offsetY=offsetY+offset.y();
    ret.x=x;
    ret.y=y;
    while(ret.offsetY>=1.0){
        ret.offsetY -= 1.0;
        ret.y++;
    }
    while(ret.offsetY<0.0){
        ret.offsetY += 1.0;
        ret.y--;
    }
    while(ret.offsetX>=1.0){
        ret.offsetX -= 1.0;
        ret.x++;
    }
    while(ret.offsetX<0.0){
        ret.offsetX += 1.0;
        ret.x--;
    }
    ret.m_hasOffset=true;
    return ret;
}
