#include "slippycoordinates.h"
#include <math.h>

//secosine for convenience
inline double sec(double in){
    return 1.0/cos(in);
}

SlippyCoordinates::SlippyCoordinates(QGeoCoordinate location, int zoom, QObject *parent):
    QObject(parent)
{
    this->m_zoom=zoom;

    // for the calculation see:
    // http://wiki.openstreetmap.org/wiki/Tilenames#X_and_Y

    double latitude=location.latitude();
    double longitude=location.longitude();

    double mercatorx = ((longitude + 180) / 360);

    double mercatory = (1.0 - (log(tan(latitude/180.0*M_PI) + sec(latitude/180.0*M_PI)) / M_PI)) / 2.0;

    setMercatorPos(QPointF(mercatorx, mercatory));


}



int SlippyCoordinates::zoom() const
{
    return m_zoom;
}




SlippyCoordinates SlippyCoordinates::operator+(QPoint offset)
{
    SlippyCoordinates res(zoom(), x(), y());
    res.settilePos(res.tilePos()+ offset);
    return res;
}

SlippyCoordinates SlippyCoordinates::operator+(QPointF offset)
{
    SlippyCoordinates ret;
    ret.setMercatorPos(MercatorPos() + offset );
    return *this;
}


bool SlippyCoordinates::operator<(const SlippyCoordinates &other) const{
    if(zoom()<other.zoom()) return true;
    if(zoom()>other.zoom()) return false;

    if(tilePos().x()<other.tilePos().x()) return true;
    if(tilePos().x()>other.tilePos().x()) return false;

    if(tilePos().y()<other.tilePos().y()) return true;
    if(tilePos().y()>other.tilePos().y()) return false;

    return false; // they are equal;
}

SlippyCoordinates::SlippyCoordinates(int zoom, int x, int y, QObject *parent):
    QObject(parent),
    m_zoom(zoom),
    m_tilePos(x,y)
{
    updateMercatorFromTiles();
}

QGeoCoordinate SlippyCoordinates::toGps(){


    double lon_deg = MercatorPos().x() * 360.0 - 180.0;

    double lat_rad = atan(sinh(M_PI * (1 - 2 * MercatorPos().y())));
    double lat_deg = lat_rad * 180.0 / M_PI;

    return QGeoCoordinate(lat_deg, lon_deg);

}
