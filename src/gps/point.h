#ifndef GPS_POINT_H
#define GPS_POINT_H

#include <QString>

namespace gps {

struct Point
{
    double latitude, longitude, height;
    unsigned timestamp;


public:
    Point(double latitude, double longitude, double height=0.0);
    double distanceTo(const Point& other);

    operator QString(){
        return QString("[%1,%2]").arg(latitude).arg(longitude);
    }



};

} // namespace gps

#endif // GPS_POINT_H
