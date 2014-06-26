#include "point.h"
#include <QDateTime>
#include <math.h>
namespace gps {

Point::Point(double latitude, double longitude, double height):
    latitude(latitude),
    longitude(longitude),
    height(height)
{
    timestamp=QDateTime::currentMSecsSinceEpoch()/1000;
}

double Point::distanceTo(const Point &other)
{
    //TODO: implement great-circle distance
}







} // namespace gps
